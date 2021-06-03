#ifndef FPMAS_PREY_PREDATOR_CONFIG_H
#define FPMAS_PREY_PREDATOR_CONFIG_H

#include "fpmas.h"
#include "yaml-cpp/yaml.h"

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
		extern double init_grass_rate;
		extern std::string model_output_file;
		extern std::string graph_output_file;
		extern Mode mode;
	}

	namespace LoadBalancing {
		extern bool enable;
		extern fpmas::api::scheduler::TimeStep period;
	}

	namespace Breakpoint {
		extern std::string load_from;
		extern bool enable;
		extern std::string file;
		extern fpmas::api::scheduler::TimeStep start;
		extern fpmas::api::scheduler::TimeStep end;
		extern fpmas::api::scheduler::Period period;
	}

	namespace Grass {
		extern int growing_rate;
	};

	namespace Prey {
		extern float reproduction_rate;
		extern int initial_energy;
		extern int move_cost;
		extern int energy_gain;
	};

	namespace Predator {
		extern float reproduction_rate;
		extern int initial_energy;
		extern int move_cost;
		extern int energy_gain;
	};
}

namespace YAML {
	template<>
		struct convert<config::Mode> {
			static Node encode(const config::Mode& mode);
			static bool decode(const Node& node, config::Mode& mode);
		};
}

void load_static_config(std::string config_file);
#endif
