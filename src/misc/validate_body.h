#pragma once

#include <string>

namespace ai
{
class Body;

struct ValidationResult
{
    std::string message;
    bool ok;
};

ValidationResult validate_body(const Body &body,
                               const std::vector<std::string> &available_modules);
}