#include "prey_predator.h"

fpmas::random::DistributedGenerator<> rd;

namespace base {
	void Grass::grow() {
		if(!_grown) {
			grow_count_down--;
			if(grow_count_down == 0) {
				grow_count_down = config::Grass::growing_rate;
				_grown = true;
			}
		}
	}

	void Die::die() {
		if(energy() <= 0)
			kill();
		if(!alive()) {
			auto groups = this->groups();

			for(auto group : groups)
				group->remove(this);
		}
	}
	namespace prey {
		void Eat::eat() {
			FPMAS_LOGD(this->model()->getMpiCommunicator().getRank(), "PREY", "Prey %s eats", FPMAS_C_STR(this->node()->getId()));
			api::Grass* cell = this->locationCell();
			fpmas::model::AcquireGuard acquire(cell);

			if(cell->grown()) {
				cell->reset();
				this->energy()+=config::Prey::energy_gain;
			}
		}
	}
	namespace predator {
		void Eat::eat() {
			auto preys = this->reachablePreys();
			if(preys.count() > 0) {
				auto prey = preys.random();
				fpmas::model::AcquireGuard acq(prey);

				if(prey->alive()) {
					this->energy()+=config::Predator::energy_gain;
					prey->kill();
				}
			}
		}
	}

	Model::Model() {
		// Schedules agent execution
		if(config::LoadBalancing::enable)
			this->scheduler().schedule(0, config::LoadBalancing::period, this->loadBalancingJob());
		this->scheduler().schedule(0.1, 1, grow_group.jobs());
		this->scheduler().schedule(0.2, 1, move_group.jobs());
		this->scheduler().schedule(0.3, 1, eat_group.jobs());
		this->scheduler().schedule(0.4, 1, reproduce_group.jobs());
		this->scheduler().schedule(0.5, 1, die_group.jobs());
	}

	void Model::init(
			fpmas::api::model::GridCellFactory<api::Grass>& grass_factory,
			fpmas::api::model::SpatialAgentFactory<api::Grass>& prey_factory,
			fpmas::api::model::GridAgentMapping& prey_mapping,
			fpmas::api::model::SpatialAgentFactory<api::Grass>& predator_factory,
			fpmas::api::model::GridAgentMapping& predator_mapping
			) {
		typename VonNeumannGrid<api::Grass>::Builder grid(
				grass_factory, config::Grid::width, config::Grid::height
				);

		// Builds a distributed grid
		auto cells = grid.build(*this);
		for(auto grass : cells)
			grow_group.add(grass);

		// Distributed Agent Builder
		GridAgentBuilder<api::Grass> agent_builder;

		// Initializes preys (distributed process)
		agent_builder.build(
				// Build preys in this model
				*this,
				// Groups to which preys are added
				{move_group, eat_group, reproduce_group, die_group},
				// Used to generate Prey instances
				prey_factory,
				// Used to initialize Prey locations
				prey_mapping);

		// Initializes predators (distributed process)
		agent_builder.build(
				// Build predators in this model
				*this,
				// Groups to which predators are added
				{move_group, eat_group, reproduce_group, die_group},
				// Used to generate Predator instances
				predator_factory,
				// Used to initialize Predator locations
				predator_mapping);
	}
}
