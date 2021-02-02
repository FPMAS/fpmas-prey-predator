#define FPMAS_LOG_LEVEL FPMAS_LOG_DEBUG

#include "fpmas.h"
#include "output.h"
#include "classic_pp.h"
#include "constrained_pp.h"

using namespace fpmas::model;

FPMAS_JSON_SET_UP(
		GridCell::JsonBase,
		classic::Prey::JsonBase,
		classic::Predator::JsonBase,
		classic::Grass::JsonBase)

#define FPMAS_LOAD_FROM_YML(YML_FILE, CLASS, STATIC_FIELD) \
	CLASS::STATIC_FIELD = YML_FILE[#CLASS][#STATIC_FIELD].as<typeof(CLASS::STATIC_FIELD)>();

void load_static_config(std::string config_file) {
	using namespace config;

	YAML::Node config = YAML::LoadFile(config_file);

	FPMAS_LOAD_FROM_YML(config, Grass, growing_rate);

	FPMAS_LOAD_FROM_YML(config, Grid, width);
	FPMAS_LOAD_FROM_YML(config, Grid, height);

	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_steps);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_preys);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_predators);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, model_output_file);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, graph_output_file);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, mode);

	FPMAS_LOAD_FROM_YML(config, Prey, reproduction_rate);
	FPMAS_LOAD_FROM_YML(config, Prey, initial_energy);
	FPMAS_LOAD_FROM_YML(config, Prey, move_cost);
	FPMAS_LOAD_FROM_YML(config, Prey, energy_gain);

	FPMAS_LOAD_FROM_YML(config, Predator, reproduction_rate);
	FPMAS_LOAD_FROM_YML(config, Predator, initial_energy);
	FPMAS_LOAD_FROM_YML(config, Predator, move_cost);
	FPMAS_LOAD_FROM_YML(config, Predator, energy_gain);
}

int main(int argc, char** argv) {
	FPMAS_REGISTER_AGENT_TYPES(
			GridCell::JsonBase,
			classic::Prey::JsonBase,
			classic::Predator::JsonBase,
			classic::Grass::JsonBase);

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
		model->scheduler().schedule(fpmas::scheduler::sub_step_end, 1, model_output.job());
		model->scheduler().schedule(fpmas::scheduler::sub_step_end, 1, graph_output.job());
	
		// Runs the simulation
		model->runtime().run(config::ModelConfig::num_steps);

		delete model;
	}
	fpmas::finalize();
}
