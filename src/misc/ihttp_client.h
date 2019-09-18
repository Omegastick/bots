#include <future>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace SingularityTrainer
{
class IHttpClient
{
  public:
    virtual ~IHttpClient() = 0;

    virtual std::future<nlohmann::json> get(const std::string &url) = 0;
    virtual std::future<nlohmann::json> post(const std::string &url,
                                             const nlohmann::json &json) = 0;
};

IHttpClient::~IHttpClient() {}
}