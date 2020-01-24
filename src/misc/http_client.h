#include <future>
#include <list>
#include <string>

#include <curl/>
#include <nlohmann/json.hpp>

#include "misc/ihttp_client.h"

class CURL;

namespace ai
{
class HttpClient : public IHttpClient
{
  private:
    std::string proxy_host;
    long proxy_port;

    void set_proxy(CURL *curl_handle);

  public:
    HttpClient(const std::string &proxy_host = "", long proxy_port = -1);

    std::future<nlohmann::json> get(const std::string &url,
                                    const std::list<std::string> &headers = {});
    std::future<nlohmann::json> post(const std::string &url,
                                     const nlohmann::json &json = {},
                                     const std::list<std::string> &headers = {});
};
}