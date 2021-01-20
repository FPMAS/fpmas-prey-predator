#ifndef FPMAS_PREY_PREDATOR_OUTPUT_H
#define FPMAS_PREY_PREDATOR_OUTPUT_H

#include "fpmas.h"
#include "prey_predator.h"
#include <fstream>

#define PP_PRINT_CONFIG(CLASS, PARAM) \
	std::cout << "\t- " #CLASS "::" #PARAM ": " << CLASS::PARAM << std::endl;

void print_current_config();

class ModelOutput : public fpmas::api::scheduler::Task {
	private:
		fpmas::api::runtime::Runtime& runtime;
		fpmas::api::model::AgentGroup& grass_group;
		fpmas::api::model::AgentGroup& prey_predator_group;
		fpmas::communication::TypedMpi<std::size_t> mpi;
		std::ofstream output {"out.csv"};

	public:
		ModelOutput(
				fpmas::api::runtime::Runtime& runtime,
				fpmas::api::model::AgentGroup& grass_group,
				fpmas::api::model::AgentGroup& prey_predator_group)
			: runtime(runtime), grass_group(grass_group), prey_predator_group(prey_predator_group), mpi(fpmas::communication::WORLD) {
				FPMAS_ON_PROC(fpmas::communication::WORLD, 0)
					output << "time,grass,prey,predator" << std::endl;

			}

		void run() override;
};
#endif
