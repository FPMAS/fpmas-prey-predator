//#include "mock_prey_predator.h"
#include "config.h"
#include "fpmas/synchro/hard/hard_sync_mode.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(Grass, constructor) {
	Grass grass;

	ASSERT_EQ(grass.grown, true);
	ASSERT_EQ(grass.grow_count_down, Grass::growing_rate);
}

bool operator==(const Grass& g1, const Grass& g2) {
	return g1.grown == g2.grown && g1.grow_count_down == g2.grow_count_down;
}

TEST(Grass, json) {
	Grass grass;
	grass.grown = false;
	grass.grow_count_down = 2;

	nlohmann::json j = fpmas::api::utils::PtrWrapper<Grass>(&grass);

	fpmas::api::utils::PtrWrapper<Grass> unserialized_grass = j;

	ASSERT_EQ(grass, *unserialized_grass.get());

	delete unserialized_grass.get();
}

TEST(Grass, grow) {
	Grass grass;

	// No effect while the grass is already grown
	grass.grow();
	ASSERT_EQ(grass.grown, true);

	// Grass has been eaten
	grass.grown = false;

	// Grass growing procedure
	for(int i = 0; i < Grass::growing_rate; i++) {
		ASSERT_EQ(grass.grown, false);
		grass.grow();
	}
	ASSERT_EQ(grass.grown, true);

	// Ensures that internal countdowns are properly re-initialized, so that
	// the grass growing cycle works
	grass.grown = false;
	for(int i = 0; i < Grass::growing_rate; i++) {
		ASSERT_EQ(grass.grown, false);
		grass.grow();
	}
	ASSERT_EQ(grass.grown, true);
}

class PreyPredatorTest : public Test {
	protected:
		GridModel<fpmas::synchro::HardSyncMode, Grass> model;
		fpmas::model::GridCellFactory<Grass> cell_factory;
		GridType::Builder grid {cell_factory, 5, 5};
		Behavior<api::PreyPredator> move_behavior {&api::PreyPredator::move};
		Behavior<api::PreyPredator> reproduce_behavior {&api::PreyPredator::reproduce};
		Behavior<api::PreyPredator> die_behavior {&api::PreyPredator::die};
		Behavior<api::PreyPredator> eat_behavior {&api::PreyPredator::eat};
		fpmas::api::model::MoveAgentGroup<Grass>& move_group {
			model.buildMoveGroup(MOVE, move_behavior)};
		fpmas::api::model::MoveAgentGroup<Grass>& reproduce_group {
			model.buildMoveGroup(REPRODUCE, reproduce_behavior)};
		fpmas::api::model::AgentGroup& die_group {
			model.buildGroup(DIE, die_behavior)};
		fpmas::api::model::AgentGroup& eat_group {
			model.buildGroup(EAT, eat_behavior)};
		fpmas::api::model::AgentGroup& dead_group {
			model.buildGroup(DEAD)};
		// Builds a simple 5x5 grid
		std::vector<Grass*> cells = grid.build(model);

		void initAgent(
				fpmas::api::model::GridAgent<Grass>* agent,
				fpmas::api::model::DiscretePoint location = {2, 2}) {
			move_group.add(agent);
			reproduce_group.add(agent);
			die_group.add(agent);
			eat_group.add(agent);

			auto cell = find_if(
					cells.begin(), cells.end(),
					[location] (Grass* grass) -> bool {return grass->location() == location;});
			agent->initLocation(*cell);
			model.runtime().execute(move_group.distributedMoveAlgorithm().jobs());
		}
};

class PreyPredatorBaseTest : public PreyPredatorTest {
	protected:
		MockPreyPredator* mock_prey_predator = new MockPreyPredator;

		void SetUp() override {
			initAgent(mock_prey_predator);
		}
};

TEST_F(PreyPredatorBaseTest, constructor) {
	ASSERT_EQ(mock_prey_predator->alive, true);
	ASSERT_EQ(mock_prey_predator->energy, MockPreyPredator::initial_energy);
}

TEST_F(PreyPredatorBaseTest, move) {
	model.runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(),
			AnyOfArray(std::vector<fpmas::api::model::DiscretePoint>({
					{1,2}, {2,2} ,{3,2}, {2,1}, {2,3}
					})
				)
			);
	ASSERT_EQ(mock_prey_predator->energy, MockPreyPredator::initial_energy - MockPreyPredator::move_cost);
}

TEST_F(PreyPredatorBaseTest, reproduce) {
	// Ensures that the next call to reproduce will generate a new agent
	MockPreyPredator::reproduction_rate = 1.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(2));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(2));
	ASSERT_THAT(die_group.localAgents(), SizeIs(2));

	auto agents = move_group.localAgents();
	auto new_agent = find_if(
			agents.begin(), agents.end(),
			[this] (fpmas::api::model::Agent* agent) -> bool {return agent != mock_prey_predator;}
			);

	ASSERT_EQ(dynamic_cast<fpmas::api::model::GridAgent<Grass>*>(*new_agent)->locationPoint(), fpmas::api::model::DiscretePoint(2, 2));
	ASSERT_THAT((*new_agent)->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));
}

TEST_F(PreyPredatorBaseTest, no_reproduce) {
	// Prevents agent reproduction
	MockPreyPredator::reproduction_rate = 0.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(1));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(1));
	ASSERT_THAT(die_group.localAgents(), SizeIs(1));
}

TEST_F(PreyPredatorBaseTest, die) {
	model.runtime().execute(die_group.jobs());

	ASSERT_EQ(mock_prey_predator->alive, true);
	ASSERT_THAT(mock_prey_predator->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));

	// Energy goes negative
	mock_prey_predator->energy -= 2*MockPreyPredator::move_cost;

	model.runtime().execute(die_group.jobs());
	ASSERT_EQ(mock_prey_predator->alive, false);
	ASSERT_THAT(mock_prey_predator->groupIds(), ElementsAre(DEAD));
	ASSERT_THAT(dead_group.localAgents(), ElementsAre(mock_prey_predator));
}

class PreyTest : public PreyPredatorTest {
	protected:
		Prey* prey = new Prey;

		void SetUp() override {
			initAgent(prey);
		}
};

TEST_F(PreyTest, eat) {
	model.runtime().execute(eat_group.jobs());

	// Check that the Grass has been eaten
	int energy = Prey::initial_energy + Prey::energy_gain;
	ASSERT_EQ(prey->locationCell()->grown, false);
	ASSERT_EQ(prey->energy, energy);
	
	// The grass is not grown yet, so no energy gain
	model.runtime().execute(eat_group.jobs());
	ASSERT_EQ(prey->energy, energy);
}

class PredatorTest : public PreyPredatorTest {
	protected:
		Predator* predator = new Predator;
		Prey* prey = new Prey;

		void SetUp() override {
			initAgent(predator);
			initAgent(prey, {3, 2});
		}
};

TEST_F(PredatorTest, eat) {
	// Eat the prey
	ASSERT_THAT(predator->node()->outNeighbors(fpmas::api::model::PERCEPTION), ElementsAre(prey->node()));
	model.runtime().execute(eat_group.jobs());

	int energy = Predator::initial_energy + Predator::energy_gain;
	ASSERT_EQ(prey->alive, false);
	ASSERT_EQ(predator->energy, energy);

	// Predator can't eat a dead prey, even if the prey is still in its
	// perception range
	model.runtime().execute(eat_group.jobs());
	ASSERT_EQ(predator->energy, energy);
}
