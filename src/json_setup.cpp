#include "classic_pp.h"
#include "constrained_pp.h"

namespace nlohmann {
	using fpmas::api::model::AgentPtr;
	void adl_serializer<AgentPtr>::to_json(json& j, const AgentPtr& data) {
		fpmas::model::to_json<CLASSIC_TYPES, CONSTRAINED_TYPES, void>(j, data);
	}
	AgentPtr adl_serializer<AgentPtr>::from_json(const json& j) {
		return std::move(fpmas::model::from_json<CLASSIC_TYPES, CONSTRAINED_TYPES, void>(j));
	}
}
