#include "mock_prey_predator.h"

float MockPreyPredator::reproduction_rate = 0.f;
const int MockPreyPredator::initial_energy = 1;
const int MockPreyPredator::move_cost = 1;
const int MockPreyPredator::energy_gain = 1;
const PPType MockPreyPredator::agent_type = PPType::PREY;
