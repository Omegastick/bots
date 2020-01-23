#include <list>
#include <string>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

int main(int /*argc*/, char */*argv*/[]) {
    cURLpp::initialize();
    curlpp::Easy request;
    request.setOpt(new curlpp::options::Url("http://asd.com"));
    std::list<std::string> header; 
    header.push_back("Content-Type: application/octet-stream"); 
    request.setOpt(new curlpp::options::HttpHeader(header));
    request.perform();
    cURLpp::terminate();
}