#include "output.h"

void print_current_config() {
	using namespace config;
	std::cout << "Running PreyPredator model with the following configuration:" << std::endl;

	PRINT_CONFIG(ModelConfig, mode);
	PRINT_CONFIG(ModelConfig, num_steps);
	PRINT_CONFIG(ModelConfig, num_preys);
	PRINT_CONFIG(ModelConfig, num_predators);
	PRINT_CONFIG(ModelConfig, model_output_file);
	PRINT_CONFIG(ModelConfig, graph_output_file);

	PRINT_CONFIG(Breakpoint, load_from);
	PRINT_CONFIG(Breakpoint, enable);
	PRINT_CONFIG(Breakpoint, file);
	PRINT_CONFIG(Breakpoint, start);
	PRINT_CONFIG(Breakpoint, end);
	PRINT_CONFIG(Breakpoint, period);

	PRINT_CONFIG(Grid, width);
	PRINT_CONFIG(Grid, height);

	PRINT_CONFIG(Grass, growing_rate);

	PRINT_CONFIG(Prey, reproduction_rate);
	PRINT_CONFIG(Prey, initial_energy);
	PRINT_CONFIG(Prey, move_cost);
	PRINT_CONFIG(Prey, energy_gain);

	PRINT_CONFIG(Predator, reproduction_rate);
	PRINT_CONFIG(Predator, initial_energy);
	PRINT_CONFIG(Predator, move_cost);
	PRINT_CONFIG(Predator, energy_gain);
}

ModelOutput::ModelOutput(fpmas::api::model::Model& model)
	: FileOutput(config::ModelConfig::model_output_file), DistributedCsvOutput(
			model.getMpiCommunicator(), 0, *this,
			{"time", [&model] () {return model.runtime().currentDate();}},
			{"grass", [&model] () {
			std::size_t num_grass = 0;
			for(auto agent : model.getGroup(GROW).localAgents())
				if(dynamic_cast<api::Grass*>(agent)->grown())
					num_grass++;
			return num_grass;
			}},
			{"prey", [&model] () {
			std::size_t num_prey = 0;
			for(auto agent : model.getGroup(MOVE).localAgents())
				if(dynamic_cast<api::Prey*>(agent))
					num_prey++;
			return num_prey;
			}},
			{"predator", [&model] () {
			std::size_t num_predator = 0;
			for(auto agent : model.getGroup(MOVE).localAgents())
				if(dynamic_cast<api::Predator*>(agent))
					num_predator++;
			return num_predator;
			}}) {}

GraphOutput::GraphOutput(fpmas::api::model::Model& model)
	: FileOutput(fpmas::utils::format(
				config::ModelConfig::graph_output_file,
				model.getMpiCommunicator().getRank())), DistributedCsvOutput(
			model.getMpiCommunicator(), *this,
			{"time", [&model] () {return model.runtime().currentDate();}},
			{"local_nodes", [&model] () {return model.graph().getNodes().size();}},
			{"local_edges", [&model] () {return model.graph().getEdges().size();}},
			{"total_nodes", [&model] () {return model.graph().getLocationManager().getLocalNodes().size();}},
			{"total_edges", [&model] () {
			std::size_t num_edges = 0;
			for(auto node : model.graph().getLocationManager().getLocalNodes())
				num_edges += node.second->getOutgoingEdges().size();
			return num_edges;
			}}
			) {}


