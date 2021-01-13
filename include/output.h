#include "fpmas.h"
#include "prey_predator.h"
#include <fstream>

class Output : public fpmas::api::scheduler::Task {
	private:
		fpmas::api::runtime::Runtime& runtime;
		fpmas::api::model::AgentGroup& grass_group;
		fpmas::api::model::AgentGroup& prey_predator_group;
		fpmas::communication::TypedMpi<std::size_t> mpi;
		std::ofstream output {"out.csv"};

	public:
		Output(
				fpmas::api::runtime::Runtime& runtime,
				fpmas::api::model::AgentGroup& grass_group,
				fpmas::api::model::AgentGroup& prey_predator_group)
			: runtime(runtime), grass_group(grass_group), prey_predator_group(prey_predator_group), mpi(fpmas::communication::WORLD) {
				FPMAS_ON_PROC(fpmas::communication::WORLD, 0)
					output << "time,grass,prey,predator" << std::endl;

			}

		void run() override {
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
};
