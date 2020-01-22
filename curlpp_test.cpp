#include <list>
#include <string>

#include <curl/curl.h>

int main(int /*argc*/, char * /*argv*/[])
{
    curl_global_init(CURL_GLOBAL_ALL);
    const auto handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, "asd.com");
    curl_slist *headers = nullptr;
    curl_slist_append(headers, "Content-Type: application/octet-stream");
    curl_easy_perform(handle);
    curl_easy_cleanup(handle);
}