#include <future>
#include <string>

#include <curlpp/Easy.hpp>
#include <nlohmann/json_fwd.hpp>

#include "misc/ihttp_client.h"

namespace SingularityTrainer
{
class HttpClient : public IHttpClient
{
  private:
    std::string proxy_host;
    long proxy_port;

    void set_proxy(cURLpp::Easy &curl_handle);

  public:
    HttpClient(const std::string &proxy_host = "", long proxy_port = -1);

    std::future<nlohmann::json> get(const std::string &url);
    std::future<nlohmann::json> post(const std::string &url, const nlohmann::json &json);
};
}