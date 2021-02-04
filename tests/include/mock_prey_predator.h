#ifndef FPMAS_MOCK_PREY_PREDATOR_H
#define FPMAS_MOCK_PREY_PREDATOR_H

#include "gmock/gmock.h"
#include "classic_pp.h"
#include "constrained_pp.h"

#define DEFAULT_MOCK_SPECIAL_MEMBERS(CLASS)\
		CLASS(const CLASS&) : CLASS() {}\
		CLASS(CLASS&&) : CLASS() {}\
		CLASS& operator=(const CLASS&) {return *this;}\
		CLASS& operator=(CLASS&&) {return *this;}\

#define DEFAULT_MOCK_CONSTRUCTORS(CLASS)\
		CLASS() {}\
		DEFAULT_MOCK_SPECIAL_MEMBERS(CLASS)

namespace mock {
	namespace config {
		extern float reproduction_rate;
		extern const int initial_energy;
		extern const int move_cost;
		extern const int energy_gain;
	}
}

namespace base {
	class MockGrass : public Grass, public fpmas::model::GridCellBase<MockGrass> {
		using Grass::Grass;
		using fpmas::model::GridCellBase<MockGrass>::GridCellBase;
	};

	class MockPreyPredator :
		public PreyPredator<MockPreyPredator> {
		public:
			MockPreyPredator()
				: PreyPredator<MockPreyPredator>(
						mock::config::initial_energy) {}

			// Forces constructor definitions (deleted by GMock)
			DEFAULT_MOCK_SPECIAL_MEMBERS(MockPreyPredator)

			MOCK_METHOD(void, die, (), (override));
			MOCK_METHOD(void, move, (), (override));
			MOCK_METHOD(void, reproduce, (), (override));
			MOCK_METHOD(void, eat, (), (override));
			MOCK_METHOD(fpmas::model::Neighbors<api::Grass>, neighborCells, (), (override));
	};

	class MockPrey :
		public api::Prey,
		public PreyPredator<MockPrey> {
			public:
				MockPrey() : PreyPredator<MockPrey>(
						mock::config::initial_energy) {}

				DEFAULT_MOCK_SPECIAL_MEMBERS(MockPrey)

				MOCK_METHOD(void, die, (), (override));
				MOCK_METHOD(void, move, (), (override));
				MOCK_METHOD(void, reproduce, (), (override));
				MOCK_METHOD(void, eat, (), (override));
				MOCK_METHOD(fpmas::model::Neighbors<api::Grass>, neighborCells, (), (override));
	};
}
#endif
