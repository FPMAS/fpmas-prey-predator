#include "constrained_pp.h"

namespace constrained {
	void Move::move() {
		this->energy() -= move_cost;
		fpmas::model::Neighbor<api::Grass> next_location;
		auto locations = this->neighborCells().shuffle();
		{
			for(auto location : locations) {
				if(location != this->locationCell()) {
					fpmas::model::AcquireGuard acq1(location);
					Grass* grass = static_cast<Grass*>(location.agent());
					if(!grass->state[_agent_type]) {
						grass->state[_agent_type] = true;
						next_location = location;
						break;
					}
				}
			}
		}
		if(next_location.agent() != nullptr) {
			{
				fpmas::model::AcquireGuard acq0(this->locationCell());
				static_cast<Grass*>(this->locationCell())
					->state[_agent_type] = false;
			}
			this->moveTo(next_location);
		}
	}

	void Reproduce::reproduce() {
		if(this->random_real(rd) <= reproduction_rate) {
			//std::cout << this->node()->getId() << " reproduces" << std::endl;
			fpmas::model::Neighbor<api::Grass> child_location;
			auto locations = this->neighborCells().shuffle();
			for(auto location : locations) {
				if(location != this->locationCell()) {
					fpmas::model::AcquireGuard acq1(location);
					Grass* grass = static_cast<Grass*>(location.agent());
					if(!grass->state[_agent_type]) {
						grass->state[_agent_type] = true;
						child_location = location;
						break;
					}
				}
			}

			if(child_location.agent() != nullptr) {
				api::PreyPredator* agent = this->agentFactory().build();
				for(auto group : this->groups())
					group->add(agent);
				agent->initLocation(child_location);
			}
		}
	}


	void Prey::to_json(nlohmann::json& j, const Prey* prey) {
		PreyPredator::to_json(j, prey);
	}

	Prey* Prey::from_json(const nlohmann::json& j) {
		Prey* prey = new Prey;
		PreyPredator::from_json(j, prey);
		return prey;
	}

	void Predator::to_json(nlohmann::json& j, const Predator* predator) {
		PreyPredator::to_json(j, predator);
	}

	Predator* Predator::from_json(const nlohmann::json& j) {
		Predator* predator = new Predator;
		PreyPredator::from_json(j, predator);
		return predator;
	}

	void Model::init() {
		ConstrainedGridAgentMapping prey_mapping(
				config::Grid::width,
				config::Grid::height,
				config::ModelConfig::num_preys,
				1
				);
		ConstrainedGridAgentMapping predator_mapping(
				config::Grid::width,
				config::Grid::height,
				config::ModelConfig::num_predators,
				1
				);

		base::GrassFactory<Grass> grass_factory;
		DefaultSpatialAgentFactory<Prey> prey_factory;
		DefaultSpatialAgentFactory<Predator> predator_factory;

		this->base::Model::init(
				grass_factory,
				prey_factory,
				prey_mapping,
				predator_factory,
				predator_mapping);
	}
}
