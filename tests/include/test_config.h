#ifndef FPMAS_PREY_PREDATOR_TEST_CONFIG_H
#define FPMAS_PREY_PREDATOR_TEST_CONFIG_H

#include "mock_prey_predator.h"
#include "fpmas.h"

template<typename GrassType>
class PreyPredatorTest : public ::testing::Test, public base::ModelBase {
	protected:
		base::GrassFactory<GrassType> grass_factory;
		typename fpmas::model::VonNeumannGrid<api::Grass>::Builder grid {
			grass_factory, 5, 5};

		fpmas::api::model::MoveAgentGroup<api::Grass>& move_group {
			this->buildMoveGroup(MOVE, this->move_behavior)};
		fpmas::api::model::MoveAgentGroup<api::Grass>& reproduce_group {
			this->buildMoveGroup(REPRODUCE, this->reproduce_behavior)};
		fpmas::api::model::AgentGroup& die_group {
			this->buildGroup(DIE, this->die_behavior)};
		fpmas::api::model::AgentGroup& eat_group {
			this->buildGroup(EAT, this->eat_behavior)};
		// Builds a simple 5x5 grid
		std::vector<api::Grass*> cells = grid.build(*this);

		void initAgent(
				fpmas::api::model::GridAgent<api::Grass>* agent,
				fpmas::api::model::DiscretePoint location = {2, 2}) {
			move_group.add(agent);
			reproduce_group.add(agent);
			die_group.add(agent);
			eat_group.add(agent);

			auto cell = find_if(
					cells.begin(), cells.end(),
					[location] (api::Grass* grass) -> bool {return grass->location() == location;});
			agent->initLocation(*cell);
			this->runtime().execute(move_group.distributedMoveAlgorithm().jobs());
		}

};

#endif
