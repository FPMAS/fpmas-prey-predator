#ifndef FPMAS_PREY_PREDATOR_H
#define FPMAS_PREY_PREDATOR_H

#include "config.h"

#include <algorithm>

FPMAS_DEFINE_GROUPS(MOVE, EAT, REPRODUCE, DIE, GROW)

using namespace fpmas::model;

extern fpmas::random::DistributedGenerator<> rd;

namespace api {
	class Grass : public virtual fpmas::api::model::GridCell {
		public:
			virtual void grow() = 0;
			virtual bool grown() const = 0;
			virtual int growCountDown() const = 0;
			virtual void reset() = 0;
	};

	class PreyPredator;
	class AgentFactory {
		public:
			virtual PreyPredator* build() const = 0;
	};

	class PreyPredator : public virtual fpmas::api::model::GridAgent<Grass> {
		public:
			virtual void move() = 0;
			virtual void eat() = 0;
			virtual void reproduce() = 0;
			virtual void die() = 0;

			virtual bool alive() const = 0;
			virtual void kill() = 0;
			virtual const int& energy() const = 0;
			virtual int& energy() = 0;

			virtual const AgentFactory& agentFactory() const = 0;

			virtual fpmas::model::Neighbors<Grass> neighborCells() = 0;
	};
	class Prey : public virtual PreyPredator {
	};

	class Predator : public virtual PreyPredator {
		public:
			virtual fpmas::model::Neighbors<Prey> reachablePreys() = 0;
	};
}

namespace base {
	class Grass : public api::Grass {
		protected:
			bool _grown;
			int grow_count_down;

		public:
			Grass() : Grass(true, config::Grass::growing_rate) {}
			
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
				: _grown(grown), grow_count_down(grow_count_down) {}

			void grow() override;

			bool grown() const override {
				return _grown;
			}
			int growCountDown() const override {
				return grow_count_down;
			}

			void reset() override {
				_grown = false;
			}
	};

	template<typename AgentType>
		class AgentFactory : public api::AgentFactory {
			public:
				api::PreyPredator* build() const override {
					return new AgentType;
				}
		};

	template<typename AgentType>
		class PreyPredator : public virtual api::PreyPredator, public fpmas::model::GridAgent<AgentType, api::Grass> {
			private:
				bool _alive = true;
				int _energy = AgentType::config::initial_energy;
				AgentFactory<AgentType> agent_factory;

			protected:
				static const VonNeumannRange<VonNeumannGrid<api::Grass>> range;
				PreyPredator(int initial_energy) :
					fpmas::model::GridAgent<AgentType, api::Grass>(this->range, this->range),
					_energy(initial_energy) {}

			public:
				bool alive() const override {
					return _alive;
				}

				void kill() override {
					_alive=false;
				}

				const int& energy() const override {
					return _energy;
				}
				int& energy() override {
					return _energy;
				}

				const api::AgentFactory& agentFactory() const override {
					return agent_factory;
				}

				static void to_json(nlohmann::json& j, const PreyPredator* agent) {
					j["alive"] = agent->_alive;
					j["energy"] = agent->_energy;
				}

				static void from_json(const nlohmann::json& j, PreyPredator* agent) {
					agent->_alive = j.at("alive").get<bool>();
					agent->_energy = j.at("energy").get<int>();
				}

				fpmas::model::Neighbors<api::Grass> neighborCells() override {
					return this->mobilityField();
				}
		};

	template<typename AgentType>
	class Predator : public virtual api::Predator, public PreyPredator<AgentType> {
		public:
			using base::PreyPredator<AgentType>::PreyPredator;

			fpmas::model::Neighbors<api::Prey> reachablePreys() override {
				return this->template perceptions<api::Prey>();
			}
	};

	class Die : public virtual api::PreyPredator {
		void die() override;
	};

	namespace prey {
		class Eat : public api::Prey {
			void eat() override;
		};
	}

	namespace predator {
		class Eat : public virtual api::Predator {
			public:
			void eat() override;
		};
	}

	template<typename AgentType>
		const VonNeumannRange<VonNeumannGrid<api::Grass>> PreyPredator<AgentType>::range(1);

	template<typename GrassType>
		class GrassFactory : public fpmas::api::model::GridCellFactory<api::Grass> {
			api::Grass* build(fpmas::api::model::DiscretePoint point) {
				return new GrassType(point);
			}
		};

	class ModelBase : public fpmas::model::GridModel<fpmas::synchro::HardSyncMode, api::Grass> {
		protected:
			// Initializes agent behaviors.
			// Preys and predator are mixed in each group.
			Behavior<api::Grass> grow_behavior {&api::Grass::grow};
			Behavior<api::PreyPredator> move_behavior{
				&api::PreyPredator::move};
			Behavior<api::PreyPredator> eat_behavior{
				&api::PreyPredator::eat};
			Behavior<api::PreyPredator> reproduce_behavior{
				&api::PreyPredator::reproduce};
			Behavior<api::PreyPredator> die_behavior{
				&api::PreyPredator::die};

			fpmas::api::model::AgentGroup& grow_group {buildGroup(GROW, this->grow_behavior)};
			fpmas::api::model::MoveAgentGroup<api::Grass>& move_group {buildMoveGroup(MOVE, this->move_behavior)};
			fpmas::api::model::AgentGroup& eat_group {buildGroup(EAT, this->eat_behavior)};
			fpmas::api::model::MoveAgentGroup<api::Grass>& reproduce_group {buildMoveGroup(REPRODUCE, this->reproduce_behavior)};
			fpmas::api::model::AgentGroup& die_group {buildGroup(DIE, this->die_behavior)};
	};

	class Model : public ModelBase {
		public:
			Model();

			void init(
					fpmas::api::model::GridCellFactory<api::Grass>& grass_factory,
					fpmas::api::model::SpatialAgentFactory<api::Grass>& prey_factory,
					fpmas::api::model::GridAgentMapping& prey_mapping,
					fpmas::api::model::SpatialAgentFactory<api::Grass>& predator_factory,
					fpmas::api::model::GridAgentMapping& predator_mapping
					);

			virtual void init() = 0;
	};
}
#endif
