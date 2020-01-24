#include <cstdlib>
#include <future>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>

#include <curl/curl.h>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "http_client.h"

namespace ai
{
HttpClient::HttpClient(const std::string &proxy_host, long proxy_port)
    : proxy_host(proxy_host),
      proxy_port(proxy_port) {}

void HttpClient::set_proxy(CURL *curl_handle)
{
    if (!proxy_host.empty() && proxy_port >= 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy_host.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, proxy_port);
    }
}

std::future<nlohmann::json> HttpClient::get(const std::string &url,
                                            const std::list<std::string> &headers)
{
    if (url.empty())
    {
        throw std::runtime_error("Input URL is empty");
    }

    return std::async(
        std::launch::async,
        [=]() {
            const auto *handle = curl_easy_init();
            curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

            std::ostringstream response;
            curl_easy_setopt(handle,
                             CURLOPT_WRITEINFO,
                             [&](char *data, size_t size, size_t, void *) {
                                 response.write(data, size);
                             });

            curl_slist *headers_slist;
            for (auto header : headers)
            {
                curl_slist_append(headers_slist, header);
            }
            curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers_slsist);

            curl_easy_perform(handle);
            curl_easy_cleanup(handle);

            nlohmann::json json;
            try
            {
                json = nlohmann::json::parse(response.str());
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
    return std::async(
        std::launch::async,
        [=]() {
            auto _headers = headers;
            _headers.push_back("Content-Type: application/json");

            const auto *handle = curl_easy_init();
            curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

            std::ostringstream response;
            curl_easy_setopt(handle,
                             CURLOPT_WRITEINFO,
                             [&](char *data, size_t size, size_t, void *) {
                                 response.write(data, size);
                             });

            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, body.c_str());

            curl_slist *headers_slist;
            for (auto header : headers)
            {
                curl_slist_append(headers_slist, header);
            }
            curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers_slsist);

            curl_easy_perform(handle);
            curl_easy_cleanup(handle);

            nlohmann::json json;
            try
            {
                json = nlohmann::json::parse(response.str());
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

            DOCTEST_CHECK(response["url"] == "https://httpbin.org/get");
        }

        SUBCASE("Sends specified headers")
        {
            std::list<std::string> headers;
            headers.push_back("Asd: 123");
            headers.push_back("Sdf: 234");
            auto response = client.get("https://httpbin.org/get", headers).get();

            DOCTEST_CHECK(response["headers"]["Asd"] == "123");
            DOCTEST_CHECK(response["headers"]["Sdf"] == "234");
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

            DOCTEST_CHECK(response["json"] == nlohmann::json({{"foo", "bar"}}));
        }

        SUBCASE("Sends specified headers")
        {
            std::list<std::string> headers;
            headers.push_back("Asd: 123");
            headers.push_back("Sdf: 234");
            auto response = client.post("https://httpbin.org/post", {}, headers).get();

            DOCTEST_CHECK(response["headers"]["Asd"] == "123");
            DOCTEST_CHECK(response["headers"]["Sdf"] == "234");
        }
    }
}
}