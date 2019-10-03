#include <future>
#include <string>

#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

namespace SingularityTrainer
{
class IHttpClient
{
  public:
    virtual ~IHttpClient() = 0;

    virtual std::future<nlohmann::json> get(const std::string &url) = 0;
    virtual std::future<nlohmann::json> post(const std::string &url,
                                             const nlohmann::json &json = {}) = 0;
};

inline IHttpClient::~IHttpClient() {}

class MockHttpClient : public trompeloeil::mock_interface<IHttpClient>
{
    IMPLEMENT_MOCK1(get);
    IMPLEMENT_MOCK2(post);
};
}