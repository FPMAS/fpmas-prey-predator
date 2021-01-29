#include "prey_predator.h"

namespace config {
	int Grid::width = 100;
	int Grid::height = 100;

	int ModelConfig::num_steps = 100;
	std::size_t ModelConfig::num_preys = 100;
	std::size_t ModelConfig::num_predators = 20;
	Mode ModelConfig::mode = CLASSIC;

	std::string ModelConfig::model_output_file = "model.csv";
	std::string ModelConfig::graph_output_file = "graph.*.csv";

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
