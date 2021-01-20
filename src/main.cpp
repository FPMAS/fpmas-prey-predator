#define FPMAS_LOG_LEVEL FPMAS_LOG_DEBUG

#include "fpmas.h"
#include "output.h"
#include "yaml-cpp/yaml.h"

using namespace fpmas::model;

FPMAS_JSON_SET_UP(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase)

#define FPMAS_LOAD_FROM_YML(YML_FILE, CLASS, STATIC_FIELD) \
	CLASS::STATIC_FIELD = YML_FILE[#CLASS][#STATIC_FIELD].as<typeof(CLASS::STATIC_FIELD)>();

void load_static_config(std::string config_file) {
	YAML::Node config = YAML::LoadFile(config_file);

	FPMAS_LOAD_FROM_YML(config, Grass, growing_rate);

	FPMAS_LOAD_FROM_YML(config, Grid, width);
	FPMAS_LOAD_FROM_YML(config, Grid, height);

	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_steps);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_preys);
	FPMAS_LOAD_FROM_YML(config, ModelConfig, num_predators);

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
	FPMAS_REGISTER_AGENT_TYPES(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase);

	if(argc == 1)
		load_static_config("config.yml");
	else
		load_static_config(argv[1]);

	print_current_config();

	fpmas::init(argc, argv);
	{
		// Defines model and environment
		GridModel<fpmas::synchro::HardSyncMode, Grass> model;
		fpmas::model::GridCellFactory<Grass> cell_factory;
		GridType::Builder grid(cell_factory, Grid::width, Grid::height);

		// Builds a distributed grid
		auto cells = grid.build(model);

		Behavior<Grass> grow_behavior(&Grass::grow);
		auto& grow = model.buildGroup(GROW, grow_behavior);
		for(auto grass : cells)
			grow.add(grass);


		// Initializes agent groups and behaviors.
		// Preys and predator are mixed in each group.
		Behavior<api::PreyPredator> move_behavior(
				&api::PreyPredator::move);
		auto& move = model.buildMoveGroup(MOVE, move_behavior);

		Behavior<api::PreyPredator> eat_behavior(
				&api::PreyPredator::eat);
		auto& eat = model.buildGroup(EAT, eat_behavior);

		Behavior<api::PreyPredator> reproduce_behavior(
				&api::PreyPredator::reproduce);
		auto& reproduce = model.buildMoveGroup(REPRODUCE, reproduce_behavior);

		Behavior<api::PreyPredator> die_behavior (
				&api::PreyPredator::die);
		auto& die = model.buildGroup(DIE, die_behavior);

		model.buildGroup(DEAD);

		// Distributed Agent Builder
		GridAgentBuilder<Grass> agent_builder;

		// Initializes preys (distributed process)
		DefaultSpatialAgentFactory<Prey> prey_factory;
		UniformGridAgentMapping prey_mapping(Grid::width, Grid::height, ModelConfig::num_preys);
		agent_builder.build(model, {move, eat, reproduce, die}, prey_factory, prey_mapping);

		// Initializes predators (distributed process)
		DefaultSpatialAgentFactory<Predator> predator_factory;
		UniformGridAgentMapping predator_mapping(Grid::width, Grid::height, ModelConfig::num_predators);
		agent_builder.build(model, {move, eat, reproduce, die}, predator_factory, predator_mapping);

		ModelOutput output_task(model.runtime(), grow, move);
		fpmas::scheduler::Job output_job({output_task});
		// Schedules agent execution
		model.scheduler().schedule(0, 20, model.loadBalancingJob());
		model.scheduler().schedule(0.1, 1, grow.jobs());
		model.scheduler().schedule(0.2, 1, move.jobs());
		model.scheduler().schedule(0.3, 1, eat.jobs());
		model.scheduler().schedule(0.4, 1, reproduce.jobs());
		model.scheduler().schedule(0.5, 1, die.jobs());
		model.scheduler().schedule(0.6, 1, output_job);

		// Runs the simulation
		model.runtime().run(ModelConfig::num_steps);
	}
	fpmas::finalize();
}
