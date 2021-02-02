#ifndef FPMAS_MOCK_PREY_PREDATOR_H
#define FPMAS_MOCK_PREY_PREDATOR_H

#include "gmock/gmock.h"
#include "classic_pp.h"
#include "constrained_pp.h"

struct MockPreyPredatorConfig {
	static const api::PreyPredator::Type agent_type;
	struct config {
		static float reproduction_rate;
		static const int initial_energy;
		static const int move_cost;
		static const int energy_gain;
	};
};

namespace base {
	class MockGrass : public Grass<MockGrass> {
		using Grass<MockGrass>::Grass;
	};

	template<typename AgentType>
	class MockPreyPredatorBase :
		public PreyPredator<AgentType, MockGrass> {
		public:
			// die() is concrete, move() and reproduce() are mocked
			MOCK_METHOD(void, move, (), (override));
			MOCK_METHOD(void, reproduce, (), (override));
	};

	class MockPreyPredator :
		public MockPreyPredatorConfig,
		public MockPreyPredatorBase<MockPreyPredator> {
		public:
			// Forces constructor definitions (deleted by GMock)
			MockPreyPredator() {}
			MockPreyPredator(const MockPreyPredator&) {}
			MockPreyPredator& operator=(const MockPreyPredator&) {return *this;}
			MockPreyPredator& operator=(MockPreyPredator&&) {return *this;}
			MockPreyPredator(MockPreyPredator&&) {}

			// die() is concrete, move(), reproduce() and eat() are mocked
			MOCK_METHOD(void, eat, (), (override));

			void setEnergy(int energy) {
				this->_energy = energy;
			}
	};

	class MockPrey : public Prey<MockPreyPredatorBase<MockPrey>> {
		// die() and eat() are concrete, move() and reproduce() are mocked
		public:
			// Forces constructor definitions (deleted by GMock)
			MockPrey() {}
			MockPrey(const MockPrey&) {}
			MockPrey& operator=(const MockPrey&) {return *this;}
			MockPrey& operator=(MockPrey&&) {return *this;}
			MockPrey(MockPrey&&) {}
	};

	class MockPredator : public Predator<MockPreyPredatorBase<MockPredator>> {
		// die() and eat() are concrete, move() and reproduce() are mocked
		public:
			// Forces constructor definitions (deleted by GMock)
			MockPredator() {}
			MockPredator(const MockPredator&) {}
			MockPredator& operator=(const MockPredator&) {return *this;}
			MockPredator& operator=(MockPredator&&) {return *this;}
			MockPredator(MockPredator&&) {}
	};

}

namespace classic {

	class MockPreyPredator :
		public MockPreyPredatorConfig,
		public PreyPredator<MockPreyPredator> {
		public:
			MockPreyPredator() {}
			MockPreyPredator(const MockPreyPredator&) {}
			MockPreyPredator& operator=(const MockPreyPredator&) {return *this;}
			MockPreyPredator& operator=(MockPreyPredator&&) {return *this;}
			MockPreyPredator(MockPreyPredator&&) {}

			// die(), move() and reproduce() are concrete, eat() is mocked
			MOCK_METHOD(void, eat, (), (override));
	};
}

namespace constrained {
	class MockPreyPredator :
		public MockPreyPredatorConfig,
		public PreyPredator<MockPreyPredator> {
		public:
			MockPreyPredator() {}
			MockPreyPredator(const MockPreyPredator&) {}
			MockPreyPredator& operator=(const MockPreyPredator&) {return *this;}
			MockPreyPredator& operator=(MockPreyPredator&&) {return *this;}
			MockPreyPredator(MockPreyPredator&&) {}

			// die(), move() and reproduce() are concrete, eat() is mocked
			MOCK_METHOD(void, eat, (), (override));
	};
}
FPMAS_DEFAULT_JSON(classic::MockPreyPredator)
#endif
