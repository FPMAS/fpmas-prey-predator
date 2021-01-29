#include "mock_prey_predator.h"

namespace classic {
	float MockPreyPredator::config::reproduction_rate = 0.f;
	const int MockPreyPredator::config::initial_energy = 1;
	const int MockPreyPredator::config::move_cost = 1;
	const int MockPreyPredator::config::energy_gain = 1;

	const MockPreyPredator::Type MockPreyPredator::agent_type = MockPreyPredator::Type::PREY;
}
