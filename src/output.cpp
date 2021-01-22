#include "output.h"

void print_current_config() {
	std::cout << "Running PreyPredator model with the following configuration:" << std::endl;

	PP_PRINT_CONFIG(ModelConfig, num_steps);
	PP_PRINT_CONFIG(ModelConfig, num_preys);
	PP_PRINT_CONFIG(ModelConfig, num_predators);
	PP_PRINT_CONFIG(ModelConfig, model_output_file);
	PP_PRINT_CONFIG(ModelConfig, graph_output_file);

	PP_PRINT_CONFIG(Grid, width);
	PP_PRINT_CONFIG(Grid, height);

	PP_PRINT_CONFIG(Grass, growing_rate);

	PP_PRINT_CONFIG(Prey, reproduction_rate);
	PP_PRINT_CONFIG(Prey, initial_energy);
	PP_PRINT_CONFIG(Prey, move_cost);
	PP_PRINT_CONFIG(Prey, energy_gain);

	PP_PRINT_CONFIG(Predator, reproduction_rate);
	PP_PRINT_CONFIG(Predator, initial_energy);
	PP_PRINT_CONFIG(Predator, move_cost);
	PP_PRINT_CONFIG(Predator, energy_gain);
}

ModelOutput::ModelOutput(
		fpmas::api::model::Model& model,
		fpmas::api::model::AgentGroup& grow,
		fpmas::api::model::AgentGroup& move)
	: FileOutput(ModelConfig::model_output_file), DistributedCsvOutput(
			model.getMpiCommunicator(), 0, this->output_file,
			{"time", [&model] () {return model.runtime().currentDate();}},
			{"grass", [&grow] () {
			std::size_t num_grass = 0;
			for(auto agent : grow.localAgents())
				if(dynamic_cast<Grass*>(agent)->grown)
					num_grass++;
			return num_grass;
			}},
			{"prey", [&move] () {
			std::size_t num_prey = 0;
			for(auto agent : move.localAgents())
				if(dynamic_cast<Prey*>(agent))
					num_prey++;
			return num_prey;
			}},
			{"predator", [&move] () {
			std::size_t num_predator = 0;
			for(auto agent : move.localAgents())
				if(dynamic_cast<Predator*>(agent))
					num_predator++;
			return num_predator;
			}}) {}

std::string GraphOutput::file_name(fpmas::api::communication::MpiCommunicator& comm) {
	std::size_t replace = ModelConfig::graph_output_file.find('*');
	std::string filename = ModelConfig::graph_output_file;
	filename.replace(
			replace, 1, std::to_string(comm.getRank()));
	return filename;
}

GraphOutput::GraphOutput(fpmas::api::model::Model& model)
	: FileOutput(file_name(model.getMpiCommunicator())), DistributedCsvOutput(
			model.getMpiCommunicator(), this->output_file,
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


