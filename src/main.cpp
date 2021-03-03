#include <thread>
#include <fstream>
#include "fpmas.h"
#include "output.h"
#include "classic_pp.h"
#include "constrained_pp.h"

using namespace fpmas::model;

fpmas::model::Breakpoint breakpoint;

int main(int argc, char** argv) {
	FPMAS_REGISTER_AGENT_TYPES(CLASSIC_TYPES, CONSTRAINED_TYPES)

	if(argc == 1)
		load_static_config("config.yml");
	else
		load_static_config(argv[1]);

	print_current_config();

	fpmas::init(argc, argv);
	{
		// Instantiates a Model depending on the provided configuration
		base::Model* model;
		switch(config::ModelConfig::mode) {
			case config::CLASSIC:
				model = new classic::Model;
				break;
			case config::CONSTRAINED:
				model = new constrained::Model;
				break;
			default:
				// Unknown model type
				std::exit(1);
		}

		if(config::Breakpoint::load_from != "") {
			// Loads model from an existing breakpoint

			std::ifstream file;
			// %r in Breakpoint::load_from is replaced by the current process
			// rank
			file.open(fpmas::utils::format(
						config::Breakpoint::load_from,
						model->getMpiCommunicator().getRank()
						));
			// Load model
			breakpoint.load(file, *model);

			file.close();
		} else {
			// Initializes a new model
			model->init();
		}

		// Automatic model breakpoint
		fpmas::model::AutoBreakpoint auto_breakpoint(
				config::Breakpoint::file, breakpoint, *model
				);

		// Model output specification
		ModelOutput model_output(*model);
		GraphOutput graph_output(*model);
		model->scheduler().schedule(
				fpmas::scheduler::sub_step_end(0), 1, model_output.job());
		model->scheduler().schedule(fpmas::scheduler::sub_step_end(0), 1, graph_output.job());
		// Optional model breakpoint
		if(config::Breakpoint::enable)
			model->scheduler().schedule(
					fpmas::scheduler::sub_step_end(config::Breakpoint::start),
					config::Breakpoint::end,
					config::Breakpoint::period,
					auto_breakpoint.job());

	
		// Runs the simulation
		std::cout << "Running model from time step " << model->runtime().currentDate() << std::endl;
		model->runtime().run(config::ModelConfig::num_steps);

		delete model;
	}
	fpmas::finalize();
}
