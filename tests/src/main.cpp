#include "gtest/gtest.h"

#include "test_config.h"

int main(int argc, char **argv) {
	FPMAS_REGISTER_AGENT_TYPES(
			//GridCell::JsonBase,
			classic::Prey::JsonBase,
			classic::Predator::JsonBase,
			classic::Grass::JsonBase
			);

	fpmas::init(argc, argv);

	std::cout << "Running fpmas-prey-predator test suit" << std::endl;
	::testing::InitGoogleTest(&argc, argv);

	int result = RUN_ALL_TESTS();

	fpmas::finalize();
	return result;
}
