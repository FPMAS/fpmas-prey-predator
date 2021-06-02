#include "test_config.h"

using namespace testing;
using namespace base;

TEST(Grass, constructor) {
	MockGrass grass(true, 10);

	ASSERT_EQ(grass.grown(), true);
	ASSERT_EQ(grass.growCountDown(), 10);
}

TEST(Grass, grow) {
	MockGrass grass(true, config::Grass::growing_rate);

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

typedef PreyPredatorTest<MockGrass> BasePreyPredatorTest;

TEST_F(BasePreyPredatorTest, constructor) {
	MockPreyPredator mock_prey_predator;
	ASSERT_EQ(mock_prey_predator.alive(), true);
	ASSERT_EQ(mock_prey_predator.energy(), mock::config::initial_energy);
}

class MockDie :
	public Die,
	public PreyPredator<MockDie> {
	public:
		MockDie()
			: PreyPredator<MockDie>(mock::config::initial_energy) {}
		DEFAULT_MOCK_SPECIAL_MEMBERS(MockDie)

		MOCK_METHOD(void, move, (), (override));
		MOCK_METHOD(void, reproduce, (), (override));
		MOCK_METHOD(void, eat, (), (override));
		MOCK_METHOD(fpmas::model::Neighbors<api::Grass>, neighborCells, (), (override));
};

TEST_F(BasePreyPredatorTest, die) {
	MockDie* mock_prey_predator = new MockDie;
	this->initAgent(mock_prey_predator);
	this->runtime().execute(die_group.jobs());

	ASSERT_EQ(mock_prey_predator->alive(), true);
	ASSERT_THAT(mock_prey_predator->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));

	// Energy goes negative
	mock_prey_predator->energy() -= 2*mock::config::move_cost;

	this->runtime().execute(die_group.jobs());
	ASSERT_THAT(move_group.localAgents(), IsEmpty());
	ASSERT_THAT(reproduce_group.localAgents(), IsEmpty());
	ASSERT_THAT(eat_group.localAgents(), IsEmpty());
	ASSERT_THAT(die_group.localAgents(), IsEmpty());
}
class MockPreyEat :
	public prey::Eat,
	public PreyPredator<MockPreyEat> {
		public:
			MockPreyEat()
				: PreyPredator<MockPreyEat>(mock::config::initial_energy) {}
			DEFAULT_MOCK_SPECIAL_MEMBERS(MockPreyEat)

			MOCK_METHOD(void, die, (), (override));
			MOCK_METHOD(void, move, (), (override));
			MOCK_METHOD(void, reproduce, (), (override));
			MOCK_METHOD(Neighbors<api::Grass>, neighborCells, (), (override));
	};

TEST_F(BasePreyPredatorTest, prey_eat) {
	MockPreyEat* prey = new MockPreyEat;
	this->initAgent(prey);
	this->runtime().execute(eat_group.jobs());

	// Check that the Grass has been eaten
	int energy = mock::config::initial_energy + config::Prey::energy_gain;
	ASSERT_EQ(prey->locationCell()->grown(), false);
	ASSERT_EQ(prey->energy(), energy);

	// The grass is not grown yet, so no energy gain
	this->runtime().execute(eat_group.jobs());
	ASSERT_EQ(prey->energy(), energy);
}
class MockPredatorEat :
	public predator::Eat,
	public base::Predator<MockPredatorEat> {
		public:
			MockPredatorEat()
				: base::Predator<MockPredatorEat>(config::Predator::initial_energy) {}
			DEFAULT_MOCK_SPECIAL_MEMBERS(MockPredatorEat)

			MOCK_METHOD(void, die, (), (override));
			MOCK_METHOD(void, move, (), (override));
			MOCK_METHOD(void, reproduce, (), (override));
			MOCK_METHOD(fpmas::model::Neighbors<api::Grass>, neighborCells, (), (override));
	};

TEST_F(BasePreyPredatorTest, predator_eat) {
	MockPredatorEat* predator = new MockPredatorEat;
	MockPrey* prey = new NiceMock<MockPrey>;
	initAgent(predator);
	initAgent(prey, {3, 2});
	// Eat the prey
	ASSERT_THAT(predator->node()->outNeighbors(fpmas::api::model::PERCEPTION), ElementsAre(prey->node()));
	this->runtime().execute(eat_group.jobs());

	int energy = config::Predator::initial_energy + config::Predator::energy_gain;
	ASSERT_EQ(prey->alive(), false);
	ASSERT_EQ(predator->energy(), energy);

	// Predator can't eat a dead prey, even if the prey is still in its
	// perception range
	this->runtime().execute(eat_group.jobs());
	ASSERT_EQ(predator->energy(), energy);
}

TEST(GrassFactory, build) {
	base::GrassFactory<MockGrass> factory(0.2);

	std::size_t grown_count = 0;
	for(std::size_t i = 0; i < 10000; i++) {
		auto grass = factory.build({0, 0});
		if(grass->grown())
			grown_count++;
		delete grass;
	}
	ASSERT_NEAR(grown_count, 10000*0.2, 10000*0.01);
}
