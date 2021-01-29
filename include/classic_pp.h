#ifndef FPMAS_CLASSIC_PP_H
#define FPMAS_CLASSIC_PP_H

#include "prey_predator.h"

namespace classic {
	class Grass : public api::Grass, public fpmas::model::GridCellBase<Grass> {
		private:
			bool _grown = true;
			int grow_count_down = config::Grass::growing_rate;

		public:
			/**
			 * Uses GridCellBase(fpmas::api::model::DiscretePoint location)
			 * constructor to initialize the Grass cell location.
			 * The Grass is initialized `grown`.
			 *
			 * This constructor is intended to be used in an
			 * fpmas::api::model::GridCellFactory to generate Grass cells.
			 */
			using fpmas::model::GridCellBase<Grass>::GridCellBase;

			Grass() : fpmas::model::GridCellBase<Grass>::GridCellBase() {}

			/**
			 * Base Grass constructor.
			 *
			 * Initializes only `growing_rate`, `grown` and `grow_count_down` fields:
			 * GridCellBase specified fields, such as the cell location, are left
			 * undefined and must be initialized afterwards. This is automatically
			 * handled by the JSON serialization process, what allows this
			 * constructor to be safely used in the to_json() method below.
			 */
			Grass(bool grown, int grow_count_down)
				: _grown(grown), grow_count_down(grow_count_down) {}



			void grow() override {
				if(!_grown) {
					//std::cout << "growing grass " << this->node()->getId() << std::endl;
					grow_count_down--;
					if(grow_count_down == 0) {
						grow_count_down = config::Grass::growing_rate;
						_grown = true;
					}
				}
			};

			bool grown() const override {
				return _grown;
			}
			int growCountDown() const override {
				return grow_count_down;
			}

			void reset() override {
				_grown = false;
			}

			static void to_json(nlohmann::json& j, const Grass* grass) {
				j[0] = grass->_grown;
				j[1] = grass->grow_count_down;
			};

			static Grass* from_json(const nlohmann::json& j) {
				Grass* grass = new Grass(
						j.at(0).get<bool>(),
						j.at(1).get<int>()
						);
				return grass;
			}
	};

	typedef VonNeumannGrid<Grass> GridType;

	template<typename AgentType>
		class PreyPredator : public api::PreyPredator, public GridAgent<AgentType, Grass> {
			protected:
				static const VonNeumannRange<GridType> range;
				PreyPredator() : GridAgent<AgentType, Grass>(this->range, this->range) {}

			private:
				fpmas::random::UniformRealDistribution<> random_real {0, 1};
			public:
				bool alive = true;
				int energy = AgentType::config::initial_energy;

				Type type() const override {
					return AgentType::agent_type;
				}

				void move() override {
					energy -= AgentType::config::move_cost;
					auto new_location = this->mobilityField().random();
					//std::cout << this->node()->getId() << " moves from " << this->locationPoint() << " to " << new_location->location() << std::endl;
					this->moveTo(new_location);
				};

				void reproduce() override {
					if(random_real(rd) <= AgentType::config::reproduction_rate) {
						//std::cout << this->node()->getId() << " reproduces" << std::endl;
						AgentType* agent = new AgentType;
						for(auto group : this->groups())
							group->add(agent);
						agent->initLocation(this->locationCell());
					}
				}

				void die() override {
					if(energy <= 0)
						alive = false;
					if(!alive) {
						//std::cout << this->node()->getId() << " dies" << std::endl;
						auto groups = this->groups();

						for(auto group : groups)
							group->remove(this);
					}
				}

				static void to_json(nlohmann::json& j, const PreyPredator* agent) {
					j["alive"] = agent->alive;
					j["energy"] = agent->energy;
				}

				static void from_json(const nlohmann::json& j, PreyPredator* agent) {
					agent->alive = j.at("alive").get<bool>();
					agent->energy = j.at("energy").get<int>();
				}
		};
	template<typename AgentType>
		const VonNeumannRange<GridType> PreyPredator<AgentType>::range(1);

	class Prey : public PreyPredator<Prey> {
		public:
			static const Type agent_type = PREY;
			typedef config::Prey config;
			
			void eat() override {
				Grass* cell = this->locationCell();
				fpmas::model::AcquireGuard acquire(cell);

				if(cell->grown()) {
					//std::cout << "[prey " << this->node()->getId() << "] eats " << cell->node()->getId() << "(" << cell->node()->state() << ")" << std::endl;
					cell->reset();
					energy+=config::energy_gain;
				}
			};

			static void to_json(nlohmann::json& j, const Prey* prey) {
				PreyPredator::to_json(j, prey);
			}
			static Prey* from_json(const nlohmann::json& j) {
				Prey* prey = new Prey;
				PreyPredator::from_json(j, prey);
				return prey;
			}
	};

	class Predator : public PreyPredator<Predator> {
		public:
			static const Type agent_type = PREDATOR;
			typedef config::Predator config;

			void eat() override {
				auto preys = this->perceptions<Prey>();
				if(preys.count() > 0) {
					auto prey = preys.random();
					fpmas::model::AcquireGuard acq(prey);

					if(prey->alive) {
						//std::cout << this->node()->getId() << " eats " << prey->node()->getId() << "(" << prey->node()->state() << ")" << std::endl;
						energy+=config::energy_gain;
						prey->alive=false;
					}
				}
			};

			static void to_json(nlohmann::json& j, const Predator* predator) {
				PreyPredator::to_json(j, predator);
			}
			static Predator* from_json(const nlohmann::json& j) {
				Predator* predator = new Predator;
				PreyPredator::from_json(j, predator);
				return predator;
			}
	};

	struct Mappings {
		protected:
			UniformGridAgentMapping prey_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_preys
			};
			UniformGridAgentMapping predator_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_predators
			};
	};

	// Mappings are initialized before the base ::Model, what is required since the
	// ::Model constructor uses mappings.
	// If mappings were Model fields, they would necessarily be initialized
	// after the base ::Model, what produces a seg fault.
	class Model : private Mappings, public ::Model<GridType, Prey, Predator, Grass> {
		private:
					public:
			Model() : ::Model<GridType, Prey, Predator, Grass>(
					prey_mapping, predator_mapping) {}
	};
}
#endif
