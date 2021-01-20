#include "output.h"

void print_current_config() {
	std::cout << "Running PreyPredator model with the following configuration:" << std::endl;

	PP_PRINT_CONFIG(ModelConfig, num_steps);
	PP_PRINT_CONFIG(ModelConfig, num_preys);
	PP_PRINT_CONFIG(ModelConfig, num_predators);

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

void ModelOutput::run() {
			std::size_t num_grass = 0;
			std::size_t num_prey = 0;
			std::size_t num_predator = 0;

			for(auto agent : grass_group.localAgents())
				if(dynamic_cast<Grass*>(agent)->grown)
					num_grass++;

			for(auto agent : prey_predator_group.agents())
				switch(dynamic_cast<api::PreyPredator*>(agent)->type()) {
					case PREY:
						num_prey++;
						break;
					case PREDATOR:
						num_predator++;
						break;
				}

			std::vector<std::size_t> num_grass_vec = mpi.gather(num_grass, 0);
			std::vector<std::size_t> num_prey_vec = mpi.gather(num_prey, 0);
			std::vector<std::size_t> num_predator_vec = mpi.gather(num_predator, 0);

			FPMAS_ON_PROC(fpmas::communication::WORLD, 0) {
				num_grass = std::accumulate(num_grass_vec.begin(), num_grass_vec.end(), 0);
				num_prey = std::accumulate(num_prey_vec.begin(), num_prey_vec.end(), 0);
				num_predator = std::accumulate(num_predator_vec.begin(), num_predator_vec.end(), 0);

				output << runtime.currentDate() << "," << num_grass << "," << num_prey << "," << num_predator << std::endl;
			}
		}
