#ifndef FPMAS_PREY_PREDATOR_CONFIG_H
#define FPMAS_PREY_PREDATOR_CONFIG_H

#include "fpmas.h"
#include "yaml-cpp/yaml.h"

namespace nlohmann {
	using fpmas::api::model::AgentPtr;
	template<>
		struct adl_serializer<AgentPtr> {
			static void to_json(json& j, const AgentPtr& data);
			static AgentPtr from_json(const json& j);
		};
}

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
			static Node encode(const config::Mode& mode);
			static bool decode(const Node& node, config::Mode& mode);
		};
}

void load_static_config(std::string config_file);
#endif
