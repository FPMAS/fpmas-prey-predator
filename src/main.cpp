#define FPMAS_LOG_LEVEL FPMAS_LOG_DEBUG

#include "fpmas.h"
#include "output.h"

using namespace fpmas::model;

int grid_width = 10;
int grid_height = 10;
int num_prey = 25;
int num_predator = 17;
int num_steps = 10;

FPMAS_JSON_SET_UP(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase)


int main(int argc, char** argv) {
	FPMAS_REGISTER_AGENT_TYPES(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase);

	fpmas::init(argc, argv);
	{
		// Defines model and environment
		GridModel<fpmas::synchro::HardSyncMode, Grass> model;
		fpmas::model::GridCellFactory<Grass> cell_factory;
		GridType::Builder grid(cell_factory, grid_width, grid_height);

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
		UniformGridAgentMapping prey_mapping(grid_width, grid_height, num_prey);
		agent_builder.build(model, {move, eat, reproduce, die}, prey_factory, prey_mapping);

		// Initializes predators (distributed process)
		DefaultSpatialAgentFactory<Predator> predator_factory;
		UniformGridAgentMapping predator_mapping(grid_width, grid_height, num_predator);
		agent_builder.build(model, {move, eat, reproduce, die}, predator_factory, predator_mapping);

		Output output_task(model.runtime(), grow, move);
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
		model.runtime().run(num_steps);
	}
	fpmas::finalize();
}
