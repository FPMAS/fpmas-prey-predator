#include "gtest/gtest.h"

#include "config.h"


int main(int argc, char **argv) {
	FPMAS_REGISTER_AGENT_TYPES(GridCell::JsonBase, Prey::JsonBase, Predator::JsonBase, Grass::JsonBase, MockPreyPredator::JsonBase);

	fpmas::init(argc, argv);

	std::cout << "Running fpmas-prey-predator test suit" << std::endl;
	::testing::InitGoogleTest(&argc, argv);

	int result = RUN_ALL_TESTS();

	fpmas::finalize();
	return result;
}
