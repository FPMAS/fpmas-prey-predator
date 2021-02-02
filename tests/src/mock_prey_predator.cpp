#include "config.h"

float MockPreyPredatorConfig::config::reproduction_rate = 0.f;
const int MockPreyPredatorConfig::config::initial_energy = 1;
const int MockPreyPredatorConfig::config::move_cost = 1;
const int MockPreyPredatorConfig::config::energy_gain = 1;

const api::PreyPredator::Type MockPreyPredatorConfig::agent_type = api::PreyPredator::Type::PREY;
