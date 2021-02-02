#include "config.h"

using namespace testing;
using namespace base;

TEST(Grass, constructor) {
	Grass<MockGrass> grass;

	ASSERT_EQ(grass.grown(), true);
	ASSERT_EQ(grass.growCountDown(), config::Grass::growing_rate);
}

TEST(Grass, grow) {
	Grass<MockGrass> grass;

	// No effect while the grass is already grown
	grass.grow();
	ASSERT_EQ(grass.grown(), true);

	// Grass has been eaten
	grass.reset();

	// Grass growing procedure
	for(int i = 0; i < config::Grass::growing_rate; i++) {
		ASSERT_EQ(grass.grown(), false);
		grass.grow();
	}
	ASSERT_EQ(grass.grown(), true);

	// Ensures that internal countdowns are properly re-initialized, so that
	// the grass growing cycle works
	grass.reset();
	for(int i = 0; i < config::Grass::growing_rate; i++) {
		ASSERT_EQ(grass.grown(), false);
		grass.grow();
	}
	ASSERT_EQ(grass.grown(), true);
}


class BasePreyPredatorTest : public PreyPredatorTest<MockGrass> {
	protected:
		MockPreyPredator* mock_prey_predator = new MockPreyPredator;

		void SetUp() override {
			initAgent(mock_prey_predator);
		}
};

TEST_F(BasePreyPredatorTest, constructor) {
	ASSERT_EQ(mock_prey_predator->alive(), true);
	ASSERT_EQ(mock_prey_predator->energy(), MockPreyPredator::config::initial_energy);
}

TEST_F(BasePreyPredatorTest, die) {
	model.runtime().execute(die_group.jobs());

	ASSERT_EQ(mock_prey_predator->alive(), true);
	ASSERT_THAT(mock_prey_predator->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));

	// Energy goes negative
	mock_prey_predator->setEnergy(mock_prey_predator->energy() - 2*MockPreyPredator::config::move_cost);

	model.runtime().execute(die_group.jobs());
	ASSERT_THAT(move_group.localAgents(), IsEmpty());
	ASSERT_THAT(reproduce_group.localAgents(), IsEmpty());
	ASSERT_THAT(eat_group.localAgents(), IsEmpty());
	ASSERT_THAT(die_group.localAgents(), IsEmpty());
}

class BasePreyTest : public BasePreyPredatorTest {
	protected:
		MockPrey* prey = new MockPrey;

		void SetUp() {
			initAgent(prey);
		}
};

TEST_F(BasePreyTest, eat) {
	model.runtime().execute(eat_group.jobs());

	// Check that the Grass has been eaten
	int energy = config::Prey::initial_energy + config::Prey::energy_gain;
	ASSERT_EQ(prey->locationCell()->grown(), false);
	ASSERT_EQ(prey->energy(), energy);

	// The grass is not grown yet, so no energy gain
	model.runtime().execute(eat_group.jobs());
	ASSERT_EQ(prey->energy(), energy);
}

class BasePredatorTest : public BasePreyPredatorTest {
	protected:
		MockPredator* predator = new MockPredator;
		MockPrey* prey = new MockPrey;

		void SetUp() override {
			initAgent(predator);
			initAgent(prey, {3, 2});
		}
};

TEST_F(BasePredatorTest, eat) {
	// Eat the prey
	ASSERT_THAT(predator->node()->outNeighbors(fpmas::api::model::PERCEPTION), ElementsAre(prey->node()));
	model.runtime().execute(eat_group.jobs());

	int energy = config::Predator::initial_energy + config::Predator::energy_gain;
	ASSERT_EQ(prey->alive(), false);
	ASSERT_EQ(predator->energy(), energy);

	// Predator can't eat a dead prey, even if the prey is still in its
	// perception range
	model.runtime().execute(eat_group.jobs());
	ASSERT_EQ(predator->energy(), energy);
}
