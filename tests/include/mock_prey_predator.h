#ifndef FPMAS_MOCK_PREY_PREDATOR_H
#define FPMAS_MOCK_PREY_PREDATOR_H

#include "gmock/gmock.h"
#include "prey_predator.h"

class MockPreyPredator : public PreyPredator<MockPreyPredator> {
	public:
		static const int initial_energy;
		static const int move_cost;
		static const int energy_gain;
		static float reproduction_rate;
		static const PPType agent_type;

		MockPreyPredator() {}
		MockPreyPredator(const MockPreyPredator&) {}
		MockPreyPredator& operator=(const MockPreyPredator&) {return *this;}
		MockPreyPredator& operator=(MockPreyPredator&&) {return *this;}
		MockPreyPredator(MockPreyPredator&&) {}

		MOCK_METHOD(void, eat, (), (override));
};
FPMAS_DEFAULT_JSON(MockPreyPredator)
#endif
