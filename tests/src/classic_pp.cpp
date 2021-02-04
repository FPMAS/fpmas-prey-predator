#include "test_config.h"
#include "fpmas/synchro/hard/hard_sync_mode.h"

using namespace testing;
using namespace classic;

namespace classic {
	bool operator==(const Grass& g1, const Grass& g2) {
		return g1.grown() == g2.grown() && g1.growCountDown() == g2.growCountDown();
	}
}

TEST(ClassicGrass, json) {
	Grass grass(false, 2);

	nlohmann::json j = fpmas::api::utils::PtrWrapper<Grass>(&grass);

	fpmas::api::utils::PtrWrapper<Grass> unserialized_grass = j;

	ASSERT_EQ(grass, *unserialized_grass.get());

	delete unserialized_grass.get();
}

typedef PreyPredatorTest<Grass> ClassicPreyPredatorTest;

namespace classic {
	class MockMove :
		public Move,
		public base::PreyPredator<MockMove> {
			public:
				MockMove() :
					Move(mock::config::move_cost),
					base::PreyPredator<MockMove>(mock::config::initial_energy) {}

				DEFAULT_MOCK_SPECIAL_MEMBERS(MockMove)

					MOCK_METHOD(void, eat, (), (override));
				MOCK_METHOD(void, reproduce, (), (override));
				MOCK_METHOD(void, die, (), (override));
		};
}

TEST_F(ClassicPreyPredatorTest, move) {
	MockMove* mock_prey_predator = new MockMove;
	initAgent(mock_prey_predator);
	this->runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(),
			AnyOfArray(std::vector<fpmas::api::model::DiscretePoint>({
					{1,2}, {2,2} ,{3,2}, {2,1}, {2,3}
					})
				)
			);
	ASSERT_EQ(mock_prey_predator->energy(), mock::config::initial_energy - mock::config::move_cost);
}

namespace classic {
	class MockReproduce :
		public Reproduce,
		public base::PreyPredator<MockReproduce> {
			public:
				MockReproduce() :
					Reproduce(mock::config::reproduction_rate),
					base::PreyPredator<MockReproduce>(mock::config::initial_energy) {}

				DEFAULT_MOCK_SPECIAL_MEMBERS(MockReproduce)

					MOCK_METHOD(void, eat, (), (override));
				MOCK_METHOD(void, move, (), (override));
				MOCK_METHOD(void, die, (), (override));
		};
}

TEST_F(ClassicPreyPredatorTest, reproduce) {
	// Ensures that the next call to reproduce will generate a new agent
	mock::config::reproduction_rate = 1.f;

	MockReproduce* mock_prey_predator = new MockReproduce;
	this->initAgent(mock_prey_predator);

	this->runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(2));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(2));
	ASSERT_THAT(eat_group.localAgents(), SizeIs(2));
	ASSERT_THAT(die_group.localAgents(), SizeIs(2));

	auto agents = move_group.localAgents();
	auto new_agent = find_if(
			agents.begin(), agents.end(),
			[mock_prey_predator] (fpmas::api::model::Agent* agent) -> bool {return agent != mock_prey_predator;}
			);

	ASSERT_THAT(*new_agent, WhenDynamicCastTo<MockReproduce*>(NotNull()));
	ASSERT_EQ(dynamic_cast<api::PreyPredator*>(*new_agent)->locationPoint(), fpmas::api::model::DiscretePoint(2, 2));
	ASSERT_THAT((*new_agent)->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));
}

TEST_F(ClassicPreyPredatorTest, no_reproduce) {
	// Prevents agent reproduction
	mock::config::reproduction_rate = 0.f;

	MockReproduce* mock_prey_predator = new MockReproduce;
	this->initAgent(mock_prey_predator);

	this->runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(1));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(1));
	ASSERT_THAT(eat_group.localAgents(), SizeIs(1));
	ASSERT_THAT(die_group.localAgents(), SizeIs(1));
}
