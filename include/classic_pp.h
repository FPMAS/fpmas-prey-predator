#ifndef FPMAS_CLASSIC_PP_H
#define FPMAS_CLASSIC_PP_H

#include "prey_predator.h"

#define CLASSIC_TYPES\
	classic::Prey::JsonBase,\
	classic::Predator::JsonBase,\
	classic::Grass::JsonBase

namespace classic {
	class Grass : public base::Grass, public fpmas::model::GridCellBase<Grass> {
		public:
			// Import constructors
			using base::Grass::Grass;

			/**
			 * Uses GridCellBase(fpmas::api::model::DiscretePoint location)
			 * constructor to initialize the Grass cell location.
			 * The Grass is initialized `grown`.
			 *
			 * This constructor is intended to be used in an
			 * fpmas::api::model::GridCellFactory to generate Grass cells.
			 */
			using fpmas::model::GridCellBase<Grass>::GridCellBase;

			static void to_json(nlohmann::json& j, const Grass* grass);
			static Grass* from_json(const nlohmann::json& j);
	};

	class Move : public virtual api::PreyPredator {
		private:
			int move_cost;
		public:
			Move(int move_cost) : move_cost(move_cost) {}
			void move() override;
	};

	class Reproduce : public virtual api::PreyPredator {
		private:
			fpmas::random::UniformRealDistribution<> random_real {0, 1};
			float reproduction_rate;

		public:
			Reproduce(float reproduction_rate)
				: reproduction_rate(reproduction_rate) {}

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
					Move(config::Prey::move_cost),
					Reproduce(config::Prey::reproduction_rate) {}

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
				Move(config::Predator::move_cost),
				Reproduce(config::Predator::reproduction_rate) {}

			static void to_json(nlohmann::json& j, const Predator* predator);
			static Predator* from_json(const nlohmann::json& j);
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

	struct Factories {
		protected:
			base::GrassFactory<Grass> grass_factory;
			DefaultSpatialAgentFactory<Prey> prey_factory;
			DefaultSpatialAgentFactory<Predator> predator_factory;
	};

	// Mappings and Factories are initialized before the base::Model, what is
	// required since the base::Model constructor uses mappings.
	// If mappings and factories were Model fields, they would necessarily be
	// initialized after the base::Model, what produces a seg fault.
	class Model : private Mappings, private Factories, public base::Model {
		public:
			Model() : base::Model(
					grass_factory,
					prey_factory,
					prey_mapping,
					predator_factory,
					predator_mapping) {}
	};
}
#endif
