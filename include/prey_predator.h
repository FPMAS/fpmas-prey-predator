#include "fpmas/model/spatial/von_neumann.h"
#include "fpmas/random/random.h"
#include "fpmas/model/guards.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, DEAD)

using namespace fpmas::model;

class Grass;
typedef VonNeumannGrid<Grass> GridType;

namespace api {
	class PreyPredator {
		public:
			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;
	};
}

class Grass : public fpmas::model::GridCellBase<Grass> {
	public:
		static const int growing_rate = 8;
		bool grown = true;
	private:
		int grow_count_down = growing_rate;

		void grow() {
			if(!grown) {
				grow_count_down--;
				if(grow_count_down < 0)
					grow_count_down = growing_rate;
				grown = true;
			}
		};
	public:
		using fpmas::model::GridCellBase<Grass>::GridCellBase;

		static void to_json(nlohmann::json& j, const Grass* grass) {
			j[0] = grass->grown;
			j[1] = grass->grow_count_down;
		};

		static Grass* from_json(const nlohmann::json& j) {
			Grass* grass = new Grass;
			grass->grown = j.at(0).get<int>();
			grass->grow_count_down = j.at(0).get<int>();
			return grass;
		}
};

template<typename AgentType>
class PreyPredator : public api::PreyPredator, public GridAgent<AgentType, Grass> {
	protected:
		static const VonNeumannRange<GridType> range;
		PreyPredator() : GridAgent<AgentType, Grass>(this->range, this->range) {}

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
		static const int energy_gain;

		void eat() override {
			Grass* cell = this->locationCell();
			std::cout << "[prey " << this->node()->getId() << "] eats " << cell->node()->getId() << "(" << cell->node()->state() << ")" << std::endl;
			fpmas::model::AcquireGuard acquire(cell);

			if(cell->grown) {
				cell->grown = false;
				energy+=energy_gain;
			}
		};

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
const int Prey::energy_gain = 4;

class Predator : public PreyPredator<Predator> {
	public:
		static const float reproduction_rate;
		static const int initial_energy;
		static const int move_cost;
		static const int energy_gain;

		void eat() override {
			auto preys = this->perceptions<Prey>();
			if(preys.count() > 0) {
				auto prey = preys.random();
				fpmas::model::AcquireGuard acq(prey);

				if(prey->alive) {
					std::cout << this->node()->getId() << " eats " << prey->node()->getId() << "(" << prey->node()->state() << ")" << std::endl;
					energy+=energy_gain;
					prey->alive=false;
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
const int Predator::energy_gain = 20;
