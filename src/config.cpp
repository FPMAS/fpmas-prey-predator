#include "config.h"

#define LOAD_FROM_YML(YML_FILE, NAMESPACE, STATIC_FIELD) \
	if(YML_FILE[#NAMESPACE])\
		if(YML_FILE[#NAMESPACE][#STATIC_FIELD])\
			NAMESPACE::STATIC_FIELD = YML_FILE[#NAMESPACE][#STATIC_FIELD]\
				.as<typeof(NAMESPACE::STATIC_FIELD)>();

namespace config {
	int Grid::width = 100;
	int Grid::height = 100;

	int ModelConfig::num_steps = 100;
	std::size_t ModelConfig::num_preys = 100;
	std::size_t ModelConfig::num_predators = 20;
	Mode ModelConfig::mode = CLASSIC;

	std::string Breakpoint::load_from = "";
	bool Breakpoint::enable = false;
	std::string Breakpoint::file = "breakpoint.%r.%t.msgpack";
	fpmas::api::scheduler::TimeStep Breakpoint::start = 0;
	fpmas::api::scheduler::TimeStep Breakpoint::end
		= std::numeric_limits<fpmas::api::scheduler::TimeStep>::max();
	fpmas::api::scheduler::Period Breakpoint::period = 10;

	std::string ModelConfig::model_output_file = "model.csv";
	std::string ModelConfig::graph_output_file = "graph.%r.csv";

	int Grass::growing_rate = 8;

	float Prey::reproduction_rate = .05;
	int Prey::initial_energy = 4;
	int Prey::move_cost = 1;
	int Prey::energy_gain = 4;

	float Predator::reproduction_rate = .04;
	int Predator::initial_energy = 20;
	int Predator::move_cost = 1;
	int Predator::energy_gain = 20;
}

namespace YAML {
	Node convert<config::Mode>::encode(const config::Mode& mode) {
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

	bool convert<config::Mode>::decode(const Node& node, config::Mode& mode) {
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
}

void load_static_config(std::string config_file) {
	using namespace config;

	YAML::Node config = YAML::LoadFile(config_file);

	LOAD_FROM_YML(config, Grass, growing_rate);

	LOAD_FROM_YML(config, Grid, width);
	LOAD_FROM_YML(config, Grid, height);

	LOAD_FROM_YML(config, ModelConfig, num_steps);
	LOAD_FROM_YML(config, ModelConfig, num_preys);
	LOAD_FROM_YML(config, ModelConfig, num_predators);
	LOAD_FROM_YML(config, ModelConfig, model_output_file);
	LOAD_FROM_YML(config, ModelConfig, graph_output_file);
	LOAD_FROM_YML(config, ModelConfig, mode);

	LOAD_FROM_YML(config, Breakpoint, load_from);
	LOAD_FROM_YML(config, Breakpoint, enable);
	LOAD_FROM_YML(config, Breakpoint, file);
	LOAD_FROM_YML(config, Breakpoint, start);
	LOAD_FROM_YML(config, Breakpoint, end);
	LOAD_FROM_YML(config, Breakpoint, period);

	LOAD_FROM_YML(config, Prey, reproduction_rate);
	LOAD_FROM_YML(config, Prey, initial_energy);
	LOAD_FROM_YML(config, Prey, move_cost);
	LOAD_FROM_YML(config, Prey, energy_gain);

	LOAD_FROM_YML(config, Predator, reproduction_rate);
	LOAD_FROM_YML(config, Predator, initial_energy);
	LOAD_FROM_YML(config, Predator, move_cost);
	LOAD_FROM_YML(config, Predator, energy_gain);
}

