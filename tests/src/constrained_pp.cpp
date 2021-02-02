#include "config.h"
#include "fpmas/synchro/hard/hard_sync_mode.h"

using namespace testing;
using namespace constrained;

TEST(ConstrainedGrass, constructor) {
	Grass grass(false, {false, true}, 4);
	ASSERT_EQ(grass.grown(), false);
	ASSERT_EQ(grass.state.prey, false);
	ASSERT_EQ(grass.state.predator, true);
	ASSERT_EQ(grass.growCountDown(), 4);
}

namespace constrained {
	bool operator==(const Grass& g1, const Grass& g2) {
		return
			g1.grown() == g2.grown()
			&& g1.growCountDown() == g2.growCountDown()
			&& g1.state.prey == g2.state.prey
			&& g1.state.predator == g2.state.predator;
	}
}

TEST(Grass, json) {
	Grass grass(false, {true, false}, 2);

	nlohmann::json j = fpmas::api::utils::PtrWrapper<Grass>(&grass);

	fpmas::api::utils::PtrWrapper<Grass> unserialized_grass = j;

	ASSERT_EQ(grass, *unserialized_grass.get());

	delete unserialized_grass.get();
}

class ConstrainedPreyPredatorTest : public PreyPredatorTest<Grass> {
	protected:
		MockPreyPredator* mock_prey_predator = new MockPreyPredator;

		void SetUp() override {
			initAgent(mock_prey_predator);
		}
};

TEST_F(ConstrainedPreyPredatorTest, move) {
	model.runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(),
			AnyOfArray(std::vector<fpmas::api::model::DiscretePoint>({
					{1,2}, {2,2} ,{3,2}, {2,1}, {2,3}
					})
				)
			);
	ASSERT_EQ(
			mock_prey_predator->energy(),
			MockPreyPredator::config::initial_energy - MockPreyPredator::config::move_cost);

}

TEST_F(ConstrainedPreyPredatorTest, move_partial_availability) {
	// Neighbor cells set up
	for(Grass* cell : cells) {
		if(
				cell->location() == fpmas::api::model::DiscretePoint {3, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {1, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {2, 3}
				) {
			// Those cells are not available
			cell->state.prey = true;
		}
		if(cell->location() == fpmas::api::model::DiscretePoint {2, 1})
			// There is a predator, but the cell is available for a prey
			cell->state.predator = true;
	}

	model.runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(), fpmas::api::model::DiscretePoint(2,1)
			);
	ASSERT_EQ(
			mock_prey_predator->energy(),
			MockPreyPredator::config::initial_energy - MockPreyPredator::config::move_cost);
}

TEST_F(ConstrainedPreyPredatorTest, move_no_availability) {
	// Neighbor cells set up
	for(Grass* cell : cells)
		if(!(cell->location() == mock_prey_predator->locationPoint()))
			cell->state.prey = true;

	model.runtime().execute(move_group.jobs());

	ASSERT_THAT(
			mock_prey_predator->locationPoint(), fpmas::api::model::DiscretePoint(2,2)
			);
	ASSERT_EQ(
			mock_prey_predator->energy(),
			MockPreyPredator::config::initial_energy - MockPreyPredator::config::move_cost);
}

TEST_F(ConstrainedPreyPredatorTest, reproduce) {
	// Ensures that the next call to reproduce will generate a new agent
	MockPreyPredator::config::reproduction_rate = 1.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(2));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(2));
	ASSERT_THAT(die_group.localAgents(), SizeIs(2));

	auto agents = move_group.localAgents();
	auto new_agent = find_if(
			agents.begin(), agents.end(),
			[this] (fpmas::api::model::Agent* agent) -> bool {return agent != mock_prey_predator;}
			);

	ASSERT_THAT(
			dynamic_cast<fpmas::api::model::GridAgent<Grass>*>(*new_agent)->locationPoint(),
			AnyOf(
				fpmas::api::model::DiscretePoint(3, 2),
				fpmas::api::model::DiscretePoint(1, 2),
				fpmas::api::model::DiscretePoint(2, 3),
				fpmas::api::model::DiscretePoint(2, 1)
				)
			);
	ASSERT_THAT((*new_agent)->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));
}

TEST_F(ConstrainedPreyPredatorTest, reproduce_partial_availability) {
	// Ensures that the next call to reproduce will generate a new agent
	MockPreyPredator::config::reproduction_rate = 1.f;

	// Neighbor cells set up
	for(Grass* cell : cells) {
		if(
				cell->location() == fpmas::api::model::DiscretePoint {3, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {1, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {2, 3}
				) {
			// Those cells are not available
			cell->state.prey = true;
		}
		if(cell->location() == fpmas::api::model::DiscretePoint {2, 1})
			// There is a predator, but the cell is available for a prey
			cell->state.predator = true;
	}

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(2));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(2));
	ASSERT_THAT(die_group.localAgents(), SizeIs(2));


	auto agents = move_group.localAgents();
	auto new_agent = find_if(
			agents.begin(), agents.end(),
			[this] (fpmas::api::model::Agent* agent) -> bool {return agent != mock_prey_predator;}
			);

	ASSERT_THAT(
			dynamic_cast<fpmas::api::model::GridAgent<Grass>*>(*new_agent)->locationPoint(),
			fpmas::api::model::DiscretePoint(2, 1)
			);
	ASSERT_THAT((*new_agent)->groupIds(), UnorderedElementsAre(MOVE, REPRODUCE, DIE, EAT));
}

TEST_F(ConstrainedPreyPredatorTest, reproduce_no_availability) {
	// Ensures that the next call to reproduce will generate a new agent
	MockPreyPredator::config::reproduction_rate = 1.f;

	// Neighbor cells set up
	for(Grass* cell : cells) {
		if(
				cell->location() == fpmas::api::model::DiscretePoint {3, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {1, 2}
				|| cell->location() == fpmas::api::model::DiscretePoint {2, 3}
				|| cell->location() == fpmas::api::model::DiscretePoint {2, 1}
				) {
			// All neighbor cells are not available
			cell->state.prey = true;
		}
	}

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(1));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(1));
	ASSERT_THAT(die_group.localAgents(), SizeIs(1));
}

TEST_F(ConstrainedPreyPredatorTest, no_reproduce) {
	// Prevents agent reproduction
	MockPreyPredator::config::reproduction_rate = 0.f;

	model.runtime().execute(reproduce_group.jobs());

	ASSERT_THAT(move_group.localAgents(), SizeIs(1));
	ASSERT_THAT(reproduce_group.localAgents(), SizeIs(1));
	ASSERT_THAT(die_group.localAgents(), SizeIs(1));
}
