#ifndef FPMAS_CLASSIC_PP_H
#define FPMAS_CLASSIC_PP_H

#include "prey_predator.h"

namespace classic {
	class Grass : public base::Grass<Grass> {
		public:
			// Import constructors
			using base::Grass<Grass>::Grass;
			
			static void to_json(nlohmann::json& j, const Grass* grass) {
				j[0] = grass->_grown;
				j[1] = grass->grow_count_down;
			};

			static Grass* from_json(const nlohmann::json& j) {
				Grass* grass = new Grass(
						j.at(0).get<bool>(),
						j.at(1).get<int>()
						);
				return grass;
			}
	};

	template<typename AgentType>
		class PreyPredator : public base::PreyPredator<AgentType, Grass> {
			public:
				void move() override {
					this->_energy -= AgentType::config::move_cost;
					auto new_location = this->mobilityField().random();
					//std::cout << this->node()->getId() << " moves from " << this->locationPoint() << " to " << new_location->location() << std::endl;
					this->moveTo(new_location);
				};

				void reproduce() override {
					if(this->random_real(rd) <= AgentType::config::reproduction_rate) {
						//std::cout << this->node()->getId() << " reproduces" << std::endl;
						AgentType* agent = new AgentType;
						for(auto group : this->groups())
							group->add(agent);
						agent->initLocation(this->locationCell());
					}
				}

				static void to_json(nlohmann::json& j, const PreyPredator* agent) {
					j["alive"] = agent->_alive;
					j["energy"] = agent->_energy;
				}

				static void from_json(const nlohmann::json& j, PreyPredator* agent) {
					agent->_alive = j.at("alive").get<bool>();
					agent->_energy = j.at("energy").get<int>();
				}
		};

	class Prey : public base::Prey<PreyPredator<Prey>> {
		public:
			static void to_json(nlohmann::json& j, const Prey* prey) {
				PreyPredator::to_json(j, prey);
			}
			static Prey* from_json(const nlohmann::json& j) {
				Prey* prey = new Prey;
				PreyPredator::from_json(j, prey);
				return prey;
			}

	};

	class Predator : public base::Predator<PreyPredator<Predator>> {
		public:
			static void to_json(nlohmann::json& j, const Predator* predator) {
				PreyPredator::to_json(j, predator);
			}
			static Predator* from_json(const nlohmann::json& j) {
				Predator* predator = new Predator;
				PreyPredator::from_json(j, predator);
				return predator;
			}
	};

	struct Mappings {
		protected:
			UniformGridAgentMapping prey_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_preys
			};
			UniformGridAgentMapping predator_mapping {
				config::Grid::width,
					config::Grid::height,
					config::ModelConfig::num_predators
			};
	};

	// Mappings are initialized before the base::Model, what is required since the
	// base::Model constructor uses mappings.
	// If mappings were Model fields, they would necessarily be initialized
	// after the base::Model, what produces a seg fault.
	class Model : private Mappings, public base::Model<Prey, Predator, Grass> {
		public:
			Model() : base::Model<Prey, Predator, Grass>(
					prey_mapping, predator_mapping) {}
	};
}
#endif
