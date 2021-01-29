#ifndef FPMAS_PREY_PREDATOR_H
#define FPMAS_PREY_PREDATOR_H

#include "fpmas.h"
#include "yaml-cpp/yaml.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, GROW)

using namespace fpmas::model;

namespace config {
	enum Mode {
		CLASSIC, UNDEFINED
	};

	namespace Grid {
		extern int width;
		extern int height;
	}

	namespace ModelConfig {
		extern int num_steps;
		extern int num_preys;
		extern int num_predators;
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
				else
					mode = config::UNDEFINED;
				return true;
			}
		};
}

static fpmas::random::DistributedGenerator<> rd;

namespace api {
	class PreyPredator {
		public:
			enum Type {
				PREY, PREDATOR
			};

			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;
			virtual Type type() const = 0;
	};
	class Grass {
		public:
			virtual void grow() = 0;
			virtual bool grown() const = 0;
			virtual int growCountDown() const = 0;
			virtual void reset() = 0;
	};
}

template<typename Grid, typename Prey, typename Predator, typename Grass>
class Model : public fpmas::model::GridModel<fpmas::synchro::HardSyncMode, Grass> {
	private:
		fpmas::model::GridCellFactory<Grass> cell_factory;
		typename Grid::Builder grid {
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
		Model() {
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
			UniformGridAgentMapping prey_mapping(
					config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_preys);
			agent_builder.build(*this, {move, eat, reproduce, die}, prey_factory, prey_mapping);

			// Initializes predators (distributed process)
			DefaultSpatialAgentFactory<Predator> predator_factory;
			UniformGridAgentMapping predator_mapping(
					config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_predators);
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
#endif
