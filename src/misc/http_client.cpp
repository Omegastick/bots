#include <cstdlib>
#include <future>
#include <list>
#include <sstream>
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "http_client.h"

namespace SingularityTrainer
{
HttpClient::HttpClient(const std::string &proxy_host, long proxy_port)
    : proxy_host(proxy_host),
      proxy_port(proxy_port) {}

void HttpClient::set_proxy(cURLpp::Easy &curl_handle)
{
    if (!proxy_host.empty() && proxy_port >= 0)
    {
        curl_handle.setOpt(cURLpp::Options::Proxy(proxy_host));
        curl_handle.setOpt(cURLpp::Options::ProxyPort(proxy_port));
    }
}

std::future<nlohmann::json> HttpClient::get(const std::string &url,
                                            const std::list<std::string> &headers)
{
    if (url.empty())
    {
        throw std::runtime_error("Input URL is empty");
    }

    return std::async(std::launch::async,
                      [=]() {
                          curlpp::Cleanup clean;
                          curlpp::Easy request;
                          request.setOpt(new curlpp::options::Url(url));

                          std::ostringstream response;
                          request.setOpt(new curlpp::options::WriteStream(&response));

                          request.setOpt(new curlpp::options::HttpHeader(headers));

                          request.perform();

                          nlohmann::json json;
                          try
                          {
                              json = nlohmann::json::parse(std::string(response.str()));
                          }
                          catch (nlohmann::json::parse_error &error)
                          {
                              spdlog::error("Bad response: {}", response.str());
                              throw;
                          }
                          return json;
                      });
}

std::future<nlohmann::json> HttpClient::post(const std::string &url,
                                             const nlohmann::json &json,
                                             const std::list<std::string> &headers)
{
    if (url.empty())
    {
        throw std::runtime_error("Input URL is empty");
    }

    auto body = json.dump();
    return std::async(std::launch::async,
                      [=]() {
                          auto _headers = headers;
                          _headers.push_back("Content-Type: application/json");

                          curlpp::Cleanup clean;
                          curlpp::Easy request;
                          request.setOpt(new curlpp::options::Url(url));
                          request.setOpt(new curlpp::options::HttpHeader(_headers));
                          request.setOpt(new curlpp::options::PostFields(body));
                          request.setOpt(new curlpp::options::PostFieldSize(body.length()));

                          std::ostringstream response;
                          request.setOpt(new curlpp::options::WriteStream(&response));

                          request.perform();

                          nlohmann::json json;
                          try
                          {
                              json = nlohmann::json::parse(std::string(response.str()));
                          }
                          catch (nlohmann::json::parse_error &error)
                          {
                              spdlog::error("Bad response: {}", response.str());
                              throw;
                          }
                          return json;
                      });
}

TEST_CASE("HttpClient")
{
    HttpClient client;

    SUBCASE("get()")
    {
        SUBCASE("Throws on empty input")
        {
            CHECK_THROWS(client.get(""));
        }

        SUBCASE("Returns correct response from the internet")
        {
            auto response = client.get("https://httpbin.org/get").get();

            CHECK(response["url"] == "https://httpbin.org/get");
        }

        SUBCASE("Sends specified headers")
        {
            std::list<std::string> headers;
            headers.push_back("Asd: 123");
            headers.push_back("Sdf: 234");
            auto response = client.get("https://httpbin.org/get", headers).get();

            CHECK(response["headers"]["Asd"] == "123");
            CHECK(response["headers"]["Sdf"] == "234");
        }
    }

    SUBCASE("post()")
    {
        SUBCASE("Throws on empty input")
        {
            CHECK_THROWS(client.post("", {}));
        }

        SUBCASE("Returns correct response from the internet")
        {
            auto response = client.post("https://httpbin.org/post", {{"foo", "bar"}}).get();

            CHECK(response["json"] == nlohmann::json({{"foo", "bar"}}));
        }

        SUBCASE("Sends specified headers")
        {
            std::list<std::string> headers;
            headers.push_back("Asd: 123");
            headers.push_back("Sdf: 234");
            auto response = client.post("https://httpbin.org/get", {}, headers).get();

            CHECK(response["headers"]["Asd"] == "123");
            CHECK(response["headers"]["Sdf"] == "234");
        }
    }
}
}