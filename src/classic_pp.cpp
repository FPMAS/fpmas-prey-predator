#include "classic_pp.h"

namespace classic {
	void Grass::to_json(nlohmann::json& j, const Grass* grass) {
		j[0] = grass->_grown;
		j[1] = grass->grow_count_down;
	}

	Grass* Grass::from_json(const nlohmann::json& j) {
		Grass* grass = new Grass(
				j.at(0).get<bool>(),
				j.at(1).get<int>()
				);
		return grass;
	}

	void Move::move() {
		this->energy() -= move_cost;
		auto new_location = this->neighborCells().random();
		this->moveTo(new_location);
	}

	void Reproduce::reproduce() {
		if(this->random_real(rd) <= reproduction_rate) {
			//std::cout << this->node()->getId() << " reproduces" << std::endl;
			api::PreyPredator* agent = this->agentFactory().build();
			this->energy() /= 2;
			agent->energy() = this->energy();
			for(auto group : this->groups())
				group->add(agent);
			agent->initLocation(this->locationCell());
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
		UniformGridAgentMapping prey_mapping(
				config::Grid::width,
				config::Grid::height,
				config::ModelConfig::num_preys
				);
		UniformGridAgentMapping predator_mapping(
				config::Grid::width,
				config::Grid::height,
				config::ModelConfig::num_predators
				);

		base::GrassFactory<Grass> grass_factory(config::ModelConfig::init_grass_rate);
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
