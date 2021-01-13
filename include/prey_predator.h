#include "fpmas/model/spatial/von_neumann.h"
#include "fpmas/random/random.h"
#include "fpmas/model/guards.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, DEAD, GROW)

using namespace fpmas::model;

namespace Grid {
	static int width = 100;
	static int height = 100;
};

namespace ModelConfig {
	static int num_steps = 100;
	static int num_preys = 20;
	static int num_predators = 20;
};

class Grass;
typedef VonNeumannGrid<Grass> GridType;

enum PPType {
	PREY, PREDATOR
};

namespace api {
	class PreyPredator {
		public:
			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;
			virtual PPType type() = 0;
	};
}

class Grass : public fpmas::model::GridCellBase<Grass> {
	public:
		static int growing_rate;
		bool grown = true;
	private:
		int grow_count_down = growing_rate;

		/**
		 * Base Grass constructor.
		 *
		 * Initializes only `growing_rate`, `grown` and `grow_count_down` fields:
		 * GridCellBase specified fields, such as the cell location, are left
		 * undefined and must be initialized afterwards. This is automatically
		 * handled by the JSON serialization process, what allows this
		 * constructor to be safely used in the to_json() method below.
		 */
		Grass(bool grown, int grow_count_down)
			: grown(grown), grow_count_down(grow_count_down) {}

	public:
		/**
		 * Uses GridCellBase(fpmas::api::model::DiscretePoint location)
		 * constructor to initialize the Grass cell location.
		 * The Grass is initialized `grown`.
		 *
		 * This constructor is intended to be used in an
		 * fpmas::api::model::GridCellFactory to generate Grass cells.
		 */
		using fpmas::model::GridCellBase<Grass>::GridCellBase;

		void grow() {
			if(!grown) {
			//std::cout << "growing grass " << this->node()->getId() << std::endl;
				grow_count_down--;
				if(grow_count_down < 0)
					grow_count_down = growing_rate;
				grown = true;
			}
		};

		static void to_json(nlohmann::json& j, const Grass* grass) {
			j[0] = grass->grown;
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
int Grass::growing_rate = 8;

static fpmas::random::DistributedGenerator<> rd;

template<typename AgentType>
class PreyPredator : public api::PreyPredator, public GridAgent<AgentType, Grass> {
	protected:
		static const VonNeumannRange<GridType> range;
		PreyPredator() : GridAgent<AgentType, Grass>(this->range, this->range) {}

	private:
		fpmas::random::UniformRealDistribution<> random_real {0, 1};
	public:
		bool alive = true;
		int energy = AgentType::initial_energy;

		PPType type() override {
			return AgentType::agent_type;
		}

		void move() override {
			energy -= AgentType::move_cost;
			auto new_location = this->mobilityField().random();
			//std::cout << this->node()->getId() << " moves from " << this->locationPoint() << " to " << new_location->location() << std::endl;
			this->moveTo(new_location);
		};

		void reproduce() override {
			if(random_real(rd) <= AgentType::reproduction_rate) {
				//std::cout << this->node()->getId() << " reproduces" << std::endl;
				AgentType* agent = new AgentType;
				for(auto group : this->groups())
					group->add(agent);
				agent->initLocation(this->locationCell());
			}
		}

		void die() override {
			if(energy <= 0)
				alive = false;
			if(!alive) {
				//std::cout << this->node()->getId() << " dies" << std::endl;
				auto groups = this->groups();
				this->model()->getGroup(DEAD).add(this);

				for(auto group : groups)
					group->remove(this);
			}
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
		static const PPType agent_type = PREY;
		static float reproduction_rate;
		static int initial_energy;
		static int move_cost;
		static int energy_gain;

		void eat() override {
			Grass* cell = this->locationCell();
			fpmas::model::AcquireGuard acquire(cell);

			if(cell->grown) {
				//std::cout << "[prey " << this->node()->getId() << "] eats " << cell->node()->getId() << "(" << cell->node()->state() << ")" << std::endl;
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
float Prey::reproduction_rate = .05;
int Prey::initial_energy = 4;
int Prey::move_cost = 1;
int Prey::energy_gain = 4;

class Predator : public PreyPredator<Predator> {
	public:
		static const PPType agent_type = PREDATOR;
		static float reproduction_rate;
		static int initial_energy;
		static int move_cost;
		static int energy_gain;

		void eat() override {
			auto preys = this->perceptions<Prey>();
			if(preys.count() > 0) {
				auto prey = preys.random();
				fpmas::model::AcquireGuard acq(prey);

				if(prey->alive) {
					//std::cout << this->node()->getId() << " eats " << prey->node()->getId() << "(" << prey->node()->state() << ")" << std::endl;
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
float Predator::reproduction_rate = .04;
int Predator::initial_energy = 20;
int Predator::move_cost = 1;
int Predator::energy_gain = 20;
