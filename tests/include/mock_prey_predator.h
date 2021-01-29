#ifndef FPMAS_MOCK_PREY_PREDATOR_H
#define FPMAS_MOCK_PREY_PREDATOR_H

#include "gmock/gmock.h"
#include "classic_pp.h"

namespace classic {
	class MockPreyPredator : public PreyPredator<MockPreyPredator> {
		public:
			static const Type agent_type;
			struct config {
				static float reproduction_rate;
				static const int initial_energy;
				static const int move_cost;
				static const int energy_gain;
			};

			MockPreyPredator() {}
			MockPreyPredator(const MockPreyPredator&) {}
			MockPreyPredator& operator=(const MockPreyPredator&) {return *this;}
			MockPreyPredator& operator=(MockPreyPredator&&) {return *this;}
			MockPreyPredator(MockPreyPredator&&) {}

			MOCK_METHOD(void, eat, (), (override));
	};
}
FPMAS_DEFAULT_JSON(classic::MockPreyPredator)
#endif
