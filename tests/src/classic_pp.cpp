#include "config.h"
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

class ClassicPreyPredatorTest : public PreyPredatorTest<Grass> {
	protected:
		MockPreyPredator* mock_prey_predator = new MockPreyPredator;

		void SetUp() override {
			initAgent(mock_prey_predator);
		}
};

TEST_F(ClassicPreyPredatorTest, move) {
	model.runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(),
			AnyOfArray(std::vector<fpmas::api::model::DiscretePoint>({
					{1,2}, {2,2} ,{3,2}, {2,1}, {2,3}
					})
				)
			);
	ASSERT_EQ(mock_prey_predator->energy(), MockPreyPredator::config::initial_energy - MockPreyPredator::config::move_cost);
}

TEST_F(ClassicPreyPredatorTest, reproduce) {
	// Ensures that the next call to reproduce will generate a new agent
	MockPreyPredator::config::reproduction_rate = 1.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(2));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(2));
	ASSERT_THAT(eat_group.localAgents(), SizeIs(2));
	ASSERT_THAT(die_group.localAgents(), SizeIs(2));

	auto agents = move_group.localAgents();
	auto new_agent = find_if(
			agents.begin(), agents.end(),
			[this] (fpmas::api::model::Agent* agent) -> bool {return agent != mock_prey_predator;}
			);

	ASSERT_EQ(dynamic_cast<fpmas::api::model::GridAgent<Grass>*>(*new_agent)->locationPoint(), fpmas::api::model::DiscretePoint(2, 2));
	ASSERT_THAT((*new_agent)->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));
}

TEST_F(ClassicPreyPredatorTest, no_reproduce) {
	// Prevents agent reproduction
	MockPreyPredator::config::reproduction_rate = 0.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(1));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(1));
	ASSERT_THAT(eat_group.localAgents(), SizeIs(1));
	ASSERT_THAT(die_group.localAgents(), SizeIs(1));
}
