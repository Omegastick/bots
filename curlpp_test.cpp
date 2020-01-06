#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

int main(int /*argc*/, char */*argv*/[]) {
    cURLpp::initialize();
    curlpp::Easy request;
    request.setOpt(new curlpp::options::Url("http://asd.com"));
    request.perform();
    cURLpp::terminate();
}