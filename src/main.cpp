#define FPMAS_LOG_LEVEL FPMAS_LOG_DEBUG

#include "fpmas.h"
#include "output.h"
#include "classic_pp.h"
#include "constrained_pp.h"

using namespace fpmas::model;

int main(int argc, char** argv) {
	FPMAS_REGISTER_AGENT_TYPES(CLASSIC_TYPES, CONSTRAINED_TYPES)

	if(argc == 1)
		load_static_config("config.yml");
	else
		load_static_config(argv[1]);

	print_current_config();

	fpmas::init(argc, argv);
	{
		fpmas::api::model::Model* model;
		switch(config::ModelConfig::mode) {
			case config::CLASSIC:
				model = new classic::Model;
				break;
			case config::CONSTRAINED:
				model = new constrained::Model;
				break;
			default:
				std::exit(1);
		}

		// Model output specification
		ModelOutput model_output(*model);
		GraphOutput graph_output(*model);
		model->scheduler().schedule(fpmas::scheduler::sub_step_end(0), 1, model_output.job());
		model->scheduler().schedule(fpmas::scheduler::sub_step_end(0), 1, graph_output.job());
	
		// Runs the simulation
		model->runtime().run(config::ModelConfig::num_steps);

		delete model;
	}
	fpmas::finalize();
}
