#include <nlohmann/json.hpp>

#include "iagent.h"

namespace ai
{
IAgent::IAgent(const nlohmann::json &body_spec, const std::string &name)
    : body_spec(body_spec), name(name) {}

int IAgent::get_action_size() const
{
    return body_spec["num_actions"];
}

const nlohmann::json &IAgent::get_body_spec() const
{
    return body_spec;
}

int IAgent::get_observation_size() const
{
    return body_spec["num_observations"];
}
}