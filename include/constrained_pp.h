#ifndef FPMAS_CONSTRAINED_PP_H
#define FPMAS_CONSTRAINED_PP_H

#include "prey_predator.h"

namespace constrained {
	class Grass : public base::Grass<Grass> {
	public:
		struct State {
			bool prey;
			bool predator;
			bool& operator[](api::PreyPredator::Type type) {
				if(type == api::PreyPredator::PREY)
					return prey;
				return predator;
			}
			State(bool prey, bool predator)
				: prey(prey), predator(predator) {}
			State() : State(false, false) {}
		};
		using base::Grass<Grass>::Grass;

		/**
		 * Base Grass constructor.
		 *
		 * Initializes only `growing_rate`, `grown` and `grow_count_down` fields:
		 * GridCellBase specified fields, such as the cell location, are left
		 * undefined and must be initialized afterwards. This is automatically
		 * handled by the JSON serialization process, what allows this
		 * constructor to be safely used in the to_json() method below.
		 */
		Grass(bool grown, State state, int grow_count_down)
			: base::Grass<Grass>(grown, grow_count_down), state(state) {}

	public:
		State state;

		static void to_json(nlohmann::json& j, const Grass* grass) {
			j[0] = grass->_grown;
			j[1] = {grass->state.prey, grass->state.predator};
			j[2] = grass->grow_count_down;
		};

		static Grass* from_json(const nlohmann::json& j) {
			Grass* grass = new Grass(
					j.at(0).get<bool>(),
					{j.at(1).at(0).get<bool>(), j.at(1).at(1).get<bool>()},
					j.at(2).get<int>()
					);
			return grass;
		}
	};

	template<typename AgentType>
		class PreyPredator : public base::PreyPredator<AgentType, Grass> {
			public:
				void move() override {
					this->_energy -= AgentType::config::move_cost;
					fpmas::model::Neighbor<Grass> next_location;
					auto locations = this->mobilityField().shuffle();
					{
						for(auto location : locations) {
							if(location != this->locationCell()) {
								fpmas::model::AcquireGuard acq1(location);
								if(!location->state[AgentType::agent_type]) {
									location->state[AgentType::agent_type] = true;
									next_location = location;
									break;
								}
							}
						}
					}
					//std::cout << this->node()->getId() << " moves from " << this->locationPoint() << " to " << new_location->location() << std::endl;
					if(next_location.agent() != nullptr) {
						{
							fpmas::model::AcquireGuard acq0(this->locationCell());
							this->locationCell()->state[AgentType::agent_type] = false;
						}
						this->moveTo(next_location);
					}
				};

				void reproduce() override {
					if(this->random_real(rd) <= AgentType::config::reproduction_rate) {
						//std::cout << this->node()->getId() << " reproduces" << std::endl;
						fpmas::model::Neighbor<Grass> child_location;
						auto locations = this->mobilityField().shuffle();
						for(auto location : locations) {
							if(location != this->locationCell()) {
								fpmas::model::AcquireGuard acq1(location);
								if(!location->state[AgentType::agent_type]) {
									location->state[AgentType::agent_type] = true;
									child_location = location;
									break;
								}
							}
						}

						if(child_location.agent() != nullptr) {
							AgentType* agent = new AgentType;
							for(auto group : this->groups())
								group->add(agent);
							agent->initLocation(child_location);
						}
					}
				}

				static void to_json(nlohmann::json& j, const PreyPredator* agent) {
					j["alive"] = agent->_alive;
					j["energy"] = agent->_energy;
				}

				static void from_json(const nlohmann::json& j, PreyPredator* agent) {
					agent->_alive = j.at("alive").get<bool>();
					agent->_energy = j.at("energy").get<int>();
				}

		};

	class Prey : public base::Prey<PreyPredator<Prey>> {
		public:
			static void to_json(nlohmann::json& j, const Prey* prey) {
				PreyPredator::to_json(j, prey);
			}
			static Prey* from_json(const nlohmann::json& j) {
				Prey* prey = new Prey;
				PreyPredator::from_json(j, prey);
				return prey;
			}

	};

	class Predator : public base::Predator<PreyPredator<Predator>> {
		public:
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
			ConstrainedGridAgentMapping prey_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_preys,
					1
			};
			ConstrainedGridAgentMapping predator_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_predators,
					1
			};
	};

	// Mappings are initialized before the base::Model, what is required since the
	// base::Model constructor uses mappings.
	// If mappings were Model fields, they would necessarily be initialized
	// after the base::Model, what produces a seg fault.
	class Model : private Mappings, public base::Model<Prey, Predator, Grass> {
		public:
			Model() : base::Model<Prey, Predator, Grass>(
					prey_mapping, predator_mapping) {}
	};
}
#endif
