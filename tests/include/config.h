#ifndef FPMAS_PREY_PREDATOR_TEST_CONFIG_H
#define FPMAS_PREY_PREDATOR_TEST_CONFIG_H

#include "mock_prey_predator.h"
#include "fpmas.h"

FPMAS_JSON_SET_UP(
		GridCell::JsonBase,
		classic::Prey::JsonBase,
		classic::Predator::JsonBase,
		classic::Grass::JsonBase,
		classic::MockPreyPredator::JsonBase)

template<typename GrassType>
class PreyPredatorTest : public ::testing::Test {
	protected:
		GridModel<fpmas::synchro::HardSyncMode, GrassType> model;
		fpmas::model::GridCellFactory<GrassType> cell_factory;
		typename fpmas::model::VonNeumannGrid<GrassType>::Builder grid {cell_factory, 5, 5};
		Behavior<api::PreyPredator> move_behavior {&api::PreyPredator::move};
		Behavior<api::PreyPredator> reproduce_behavior {&api::PreyPredator::reproduce};
		Behavior<api::PreyPredator> die_behavior {&api::PreyPredator::die};
		Behavior<api::PreyPredator> eat_behavior {&api::PreyPredator::eat};
		fpmas::api::model::MoveAgentGroup<GrassType>& move_group {
			model.buildMoveGroup(MOVE, move_behavior)};
		fpmas::api::model::MoveAgentGroup<GrassType>& reproduce_group {
			model.buildMoveGroup(REPRODUCE, reproduce_behavior)};
		fpmas::api::model::AgentGroup& die_group {
			model.buildGroup(DIE, die_behavior)};
		fpmas::api::model::AgentGroup& eat_group {
			model.buildGroup(EAT, eat_behavior)};
		// Builds a simple 5x5 grid
		std::vector<GrassType*> cells = grid.build(model);

		void initAgent(
				fpmas::api::model::GridAgent<GrassType>* agent,
				fpmas::api::model::DiscretePoint location = {2, 2}) {
			move_group.add(agent);
			reproduce_group.add(agent);
			die_group.add(agent);
			eat_group.add(agent);

			auto cell = find_if(
					cells.begin(), cells.end(),
					[location] (GrassType* grass) -> bool {return grass->location() == location;});
			agent->initLocation(*cell);
			model.runtime().execute(move_group.distributedMoveAlgorithm().jobs());
		}

};

#endif
