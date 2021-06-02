#ifndef FPMAS_CONSTRAINED_PP_H
#define FPMAS_CONSTRAINED_PP_H

#include "prey_predator.h"

#define CONSTRAINED_TYPES\
	constrained::Prey::JsonBase,\
	constrained::Predator::JsonBase,\
	constrained::Grass::JsonBase

namespace constrained {
	enum AgentType {
		PREY, PREDATOR
	};

	class Grass : public base::Grass, public GridCellBase<Grass> {
		public:
			struct State {
				bool prey;
				bool predator;
				bool& operator[](AgentType type) {
					if(type == PREY)
						return prey;
					return predator;
				}
				State(bool prey, bool predator)
					: prey(prey), predator(predator) {}
				State() : State(false, false) {}
			};

			Grass() = default;
			Grass(bool grown, int grow_count_down)
				: base::Grass(grown, grow_count_down) {
				}

			Grass(bool grown, int grow_count_down, DiscretePoint point)
				: base::Grass(grown, grow_count_down), GridCellBase<Grass>(point) {
				}

			/**
			 * Base Grass constructor.
			 *
			 * Initializes only `growing_rate`, `grown` and `grow_count_down` fields:
			 * GridCellBase specified fields, such as the cell location, are left
			 * undefined and must be initialized afterwards. This is automatically
			 * handled by the JSON serialization process, what allows this
			 * constructor to be safely used in the to_json() method below.
			 */
			Grass(bool grown, State state, int grow_count_down)
				: base::Grass(grown, grow_count_down), state(state) {}

		public:
			State state;

			static void to_json(nlohmann::json& j, const Grass* grass) {
				j[0] = grass->_grown;
				j[1] = {grass->state.prey, grass->state.predator};
				j[2] = grass->grow_count_down;
			};

			static Grass* from_json(const nlohmann::json& j) {
				Grass* grass = new Grass(
						j.at(0).get<bool>(),
						{j.at(1).at(0).get<bool>(), j.at(1).at(1).get<bool>()},
						j.at(2).get<int>()
						);
				return grass;
			}
	};

	class Move : virtual api::PreyPredator {
		private:
			int move_cost;
			AgentType _agent_type;
		public:
			Move(int move_cost, AgentType agent_type)
				: move_cost(move_cost), _agent_type(agent_type) {}
			void move() override;
	};

	class Reproduce : public virtual api::PreyPredator {
		private:
			fpmas::random::UniformRealDistribution<> random_real {0, 1};
			AgentType _agent_type;
			float reproduction_rate;
		public:
			Reproduce(
					AgentType agent_type,
					float reproduction_rate)
				: _agent_type(agent_type), reproduction_rate(reproduction_rate) {}

		void reproduce() override;
	};

	class Prey :
		public base::PreyPredator<Prey>,
		public base::Die,
		public base::prey::Eat,
		public Move,
		public Reproduce {
		public:
			Prey() :
				base::PreyPredator<Prey>(config::Prey::initial_energy),
				Move(config::Prey::move_cost, PREY),
				Reproduce(
						PREY,
						config::Prey::reproduction_rate) {}

			static void to_json(nlohmann::json& j, const Prey* prey);
			static Prey* from_json(const nlohmann::json& j);
	};

	class Predator :
		public base::Predator<Predator>,
		public base::Die,
		public base::predator::Eat,
		public Move,
		public Reproduce {
		public:
			Predator() :
				base::Predator<Predator>(config::Predator::initial_energy),
				Move(config::Predator::move_cost, PREDATOR),
				Reproduce(
						PREDATOR,
						config::Predator::reproduction_rate) {}

			static void to_json(nlohmann::json& j, const Predator* predator);
			static Predator* from_json(const nlohmann::json& j);
	};

	class Model : public base::Model {
		public:
			void init() override;
	};
}
#endif
