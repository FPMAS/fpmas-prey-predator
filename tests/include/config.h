#ifndef FPMAS_PREY_PREDATOR_TEST_CONFIG_H
#define FPMAS_PREY_PREDATOR_TEST_CONFIG_H

#include "mock_prey_predator.h"
#include "fpmas.h"

FPMAS_JSON_SET_UP(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase, MockPreyPredator::JsonBase)
#endif
