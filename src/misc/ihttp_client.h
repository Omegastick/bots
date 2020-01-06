#include <future>
#include <list>
#include <string>

#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

namespace trompeloeil
{
template <>
inline void print(std::ostream &os, const std::list<std::string> &list)
{
    os << list.size() << "#{ ";
    for (auto item : list)
    {
        os << item << ", ";
    }
    os << "}";
}
}

namespace ai
{
const std::string st_cloud_base_url = "https://asia-northeast1-st-dev-252104.cloudfunctions.net/";

class IHttpClient
{
  public:
    virtual ~IHttpClient() = 0;

    virtual std::future<nlohmann::json> get(const std::string &url,
                                            const std::list<std::string> &headers = {}) = 0;
    virtual std::future<nlohmann::json> post(const std::string &url,
                                             const nlohmann::json &json = {},
                                             const std::list<std::string> &headers = {}) = 0;
};

inline IHttpClient::~IHttpClient() {}

class MockHttpClient : public trompeloeil::mock_interface<IHttpClient>
{
    IMPLEMENT_MOCK2(get);
    IMPLEMENT_MOCK3(post);
};
}