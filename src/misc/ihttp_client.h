#include <future>
#include <list>
#include <string>

#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

namespace SingularityTrainer
{
class IHttpClient
{
  public:
    virtual ~IHttpClient() = 0;

    virtual std::future<nlohmann::json> get(const std::string &url,
                                            std::list<std::string> headers = {}) = 0;
    virtual std::future<nlohmann::json> post(const std::string &url,
                                             const nlohmann::json &json = {},
                                             std::list<std::string> headers = {}) = 0;
};

inline IHttpClient::~IHttpClient() {}

class MockHttpClient : public trompeloeil::mock_interface<IHttpClient>
{
    IMPLEMENT_MOCK2(get);
    IMPLEMENT_MOCK3(post);
};
}