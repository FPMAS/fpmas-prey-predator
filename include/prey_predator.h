#ifndef FPMAS_PREY_PREDATOR_H
#define FPMAS_PREY_PREDATOR_H

#include "fpmas.h"
#include "yaml-cpp/yaml.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, GROW)

using namespace fpmas::model;

namespace config {
	enum Mode {
		CLASSIC, CONSTRAINED, UNDEFINED
	};

	namespace Grid {
		extern int width;
		extern int height;
	}

	namespace ModelConfig {
		extern int num_steps;
		extern std::size_t num_preys;
		extern std::size_t num_predators;
		extern std::string model_output_file;
		extern std::string graph_output_file;
		extern Mode mode;
	}

	struct Grass {
		static int growing_rate;
	};

	struct Prey {
		static float reproduction_rate;
		static int initial_energy;
		static int move_cost;
		static int energy_gain;
	};

	struct Predator {
		static float reproduction_rate;
		static int initial_energy;
		static int move_cost;
		static int energy_gain;
	};
}

namespace YAML {
	template<>
		struct convert<config::Mode> {
			static Node encode(const config::Mode& mode) {
				Node node;
				switch(mode) {
					case config::CLASSIC:
						node = "CLASSIC";
						break;
					case config::CONSTRAINED:
						node = "CONSTRAINED";
						break;
					default:
						node ="";
				}
				return node;
			}

			static bool decode(const Node& node, config::Mode& mode) {
				if(!node.IsScalar()) {
					return false;
				}
				std::string str = node.as<std::string>();
				if(str == "CLASSIC")
					mode = config::CLASSIC;
				else if (str == "CONSTRAINED")
					mode = config::CONSTRAINED;
				else
					mode = config::UNDEFINED;
				return true;
			}
		};
}

extern fpmas::random::DistributedGenerator<> rd;

namespace api {
	class PreyPredator : public virtual fpmas::api::model::Agent {
		public:
			enum Type {
				PREY, PREDATOR
			};

			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;

			virtual bool alive() const = 0;
			virtual void kill() = 0;
			virtual int energy() const = 0;

			virtual Type type() const = 0;
	};
	class Grass : public virtual fpmas::api::model::GridCell {
		public:
			virtual void grow() = 0;
			virtual bool grown() const = 0;
			virtual int growCountDown() const = 0;
			virtual void reset() = 0;
	};
}

namespace base {
	template<typename GrassType>
	class Grass : public api::Grass, public fpmas::model::GridCellBase<GrassType> {
		protected:
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
			using fpmas::model::GridCellBase<GrassType>::GridCellBase;

			Grass() : fpmas::model::GridCellBase<GrassType>::GridCellBase() {}

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
	};

	/**
	 * Base PreyPredator implementation.
	 *
	 * This is a abstract class, that only implements the die() behavior.
	 *
	 * @tparam AgentType Most derived type from this class (concrete class)
	 * @tparam GrassType Most derived api::Grass implementation
	 */
	template<typename AgentType, typename GrassType>
		class PreyPredator : public api::PreyPredator, public GridAgent<AgentType, GrassType> {
			protected:
				static const VonNeumannRange<VonNeumannGrid<GrassType>> range;
				PreyPredator() : GridAgent<AgentType, GrassType>(this->range, this->range) {}

				fpmas::random::UniformRealDistribution<> random_real {0, 1};

			public:
				bool _alive = true;
				int _energy = AgentType::config::initial_energy;

				Type type() const override {
					return AgentType::agent_type;
				}

				void die() override {
					if(energy() <= 0)
						kill();
					if(!alive()) {
						//std::cout << this->node()->getId() << " dies" << std::endl;
						auto groups = this->groups();

						for(auto group : groups)
							group->remove(this);
					}
				}

				bool alive() const override {
					return _alive;
				}

				void kill() override {
					_alive=false;
				}

				int energy() const override {
					return _energy;
				}
		};
	template<typename AgentType, typename GrassType>
		const VonNeumannRange<VonNeumannGrid<GrassType>> PreyPredator<AgentType, GrassType>::range(1);

	/**
	 * Base Prey implementation.
	 *
	 * The provided PreyPredatorType implementation must implement move(),
	 * reproduce() and die(), so that this class only implements the prey specific
	 * eat() behavior.
	 */
	template<typename PreyPredatorType>
	class Prey : public PreyPredatorType {
		public:
			static const api::PreyPredator::Type agent_type = api::PreyPredator::PREY;
			typedef config::Prey config;
			
			void eat() override {
				api::Grass* cell = this->locationCell();
				fpmas::model::AcquireGuard acquire(cell);

				if(cell->grown()) {
					//std::cout << "[prey " << this->node()->getId() << "] eats " << cell->node()->getId() << "(" << cell->node()->state() << ")" << std::endl;
					cell->reset();
					this->_energy+=config::energy_gain;
				}
			};
	};

	template<typename PreyPredatorType>
	class Predator : public PreyPredatorType {
		public:
			static const api::PreyPredator::Type agent_type = api::PreyPredator::PREDATOR;
			typedef config::Predator config;

			void eat() override {
				auto preys = this->template perceptions<api::PreyPredator>()
					.filter([] (api::PreyPredator* prey_predator) {return prey_predator->type() == api::PreyPredator::PREY;});
				if(preys.count() > 0) {
					auto prey = preys.random();
					fpmas::model::AcquireGuard acq(prey);

					if(prey->alive()) {
						//std::cout << this->node()->getId() << " eats " << prey->node()->getId() << "(" << prey->node()->state() << ")" << std::endl;
						this->_energy+=config::energy_gain;
						prey->kill();
					}
				}
			};
	};

	template<typename Prey, typename Predator, typename Grass>
		class Model : public fpmas::model::GridModel<fpmas::synchro::HardSyncMode, Grass> {
			private:
				fpmas::model::GridCellFactory<Grass> cell_factory;
				typename VonNeumannGrid<Grass>::Builder grid {
					cell_factory, config::Grid::width, config::Grid::height
				};

				Behavior<api::Grass> grow_behavior {&api::Grass::grow};

				// Initializes agent groups and behaviors.
				// Preys and predator are mixed in each group.
				Behavior<api::PreyPredator> move_behavior{
					&api::PreyPredator::move};
				Behavior<api::PreyPredator> eat_behavior{
					&api::PreyPredator::eat};
				Behavior<api::PreyPredator> reproduce_behavior{
					&api::PreyPredator::reproduce};
				Behavior<api::PreyPredator> die_behavior{
					&api::PreyPredator::die};

			public:
				Model(
						fpmas::api::model::GridAgentMapping& prey_mapping,
						fpmas::api::model::GridAgentMapping& predator_mapping
					 ) {
					// Builds a distributed grid
					auto cells = grid.build(*this);

					auto& grow = this->buildGroup(GROW, grow_behavior);
					for(auto grass : cells)
						grow.add(grass);

					auto& move = this->buildMoveGroup(MOVE, move_behavior);
					auto& eat = this->buildGroup(EAT, eat_behavior);
					auto& reproduce = this->buildMoveGroup(REPRODUCE, reproduce_behavior);
					auto& die = this->buildGroup(DIE, die_behavior);

					// Distributed Agent Builder
					GridAgentBuilder<Grass> agent_builder;

					// Initializes preys (distributed process)
					DefaultSpatialAgentFactory<Prey> prey_factory;
					agent_builder.build(*this, {move, eat, reproduce, die}, prey_factory, prey_mapping);

					// Initializes predators (distributed process)
					DefaultSpatialAgentFactory<Predator> predator_factory;
					agent_builder.build(*this, {move, eat, reproduce, die}, predator_factory, predator_mapping);

					// Schedules agent execution
					this->scheduler().schedule(0, 20, this->loadBalancingJob());
					this->scheduler().schedule(0.1, 1, grow.jobs());
					this->scheduler().schedule(0.2, 1, move.jobs());
					this->scheduler().schedule(0.3, 1, eat.jobs());
					this->scheduler().schedule(0.4, 1, reproduce.jobs());
					this->scheduler().schedule(0.5, 1, die.jobs());
				}
		};
}
#endif
