#include "fpmas/model/spatial/von_neumann.h"
#include "fpmas/random/random.h"
#include "fpmas/model/guards.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, DEAD)

using namespace fpmas::model;

typedef VonNeumannGrid GridType;

namespace api {
	class PreyPredator {
		public:
			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;
	};
}

template<typename AgentType>
class PreyPredator : public api::PreyPredator, public GridAgent<AgentType> {
	protected:
		static const VonNeumannRange<GridType> range;
		PreyPredator() : GridAgent<AgentType>(this->range, this->range) {}

	private:
		fpmas::random::DistributedGenerator<> rd;
		fpmas::random::UniformRealDistribution<> random_real {0, 1};
	public:
		bool alive = true;
		int energy = AgentType::initial_energy;

		void move() override {
			energy -= AgentType::move_cost;
			this->moveTo(this->mobilityField().random());
		};

		void reproduce() override {
			if(random_real(rd) <= AgentType::reproduction_rate) {
				AgentType* agent = new AgentType;
				for(auto group : this->groups())
					group->add(agent);
				agent->initLocation(this->locationCell());
			}
		}

		void die() override {
			if(energy <= 0)
				alive = false;
			auto groups = this->groups();
			if(!alive)
				this->model()->getGroup(DEAD).add(this);

			for(auto group : groups)
				group->remove(this);
		}

		static void to_json(nlohmann::json& j, const PreyPredator* agent) {
			j["alive"] = agent->alive;
			j["energy"] = agent->energy;
		}

		static void from_json(const nlohmann::json& j, PreyPredator* agent) {
			agent->alive = j.at("alive").get<bool>();
			agent->energy = j.at("energy").get<int>();
		}
};
template<typename AgentType>
const VonNeumannRange<GridType> PreyPredator<AgentType>::range(1);

class Prey : public PreyPredator<Prey> {
	public:
		static const float reproduction_rate;
		static const int initial_energy;
		static const int move_cost;

		void eat() override {};

		static void to_json(nlohmann::json& j, const Prey* prey) {
			PreyPredator::to_json(j, prey);
		}
		static Prey* from_json(const nlohmann::json& j) {
			Prey* prey = new Prey;
			PreyPredator::from_json(j, prey);
			return prey;
		}
};
const float Prey::reproduction_rate = .5;
const int Prey::initial_energy = 4;
const int Prey::move_cost = 1;

class Predator : public PreyPredator<Predator> {
	public:
		static const float reproduction_rate;
		static const int initial_energy;
		static const int move_cost;

		void eat() override {
			if(alive) {
			auto preys = this->perceptions<Prey>();
			if(preys.count() > 0) {
				auto prey = preys.random();
				fpmas::model::AcquireGuard acq(prey);

				if(prey->alive) {
					std::cout << this->node()->getId() << " eats " << prey->node()->getId() << "(" << prey->node()->state() << ")" << std::endl;
					energy++;
					prey->alive=false;
				}
			}
			}
		};

		static void to_json(nlohmann::json& j, const Predator* predator) {
			PreyPredator::to_json(j, predator);
		}
		static Predator* from_json(const nlohmann::json& j) {
			Predator* predator = new Predator;
			PreyPredator::from_json(j, predator);
			return predator;
		}
};
const float Predator::reproduction_rate = .4;
const int Predator::initial_energy = 20;
const int Predator::move_cost = 1;
