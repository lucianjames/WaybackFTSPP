#include <string>
#include <vector>

#include <curl/curl.h>

#include "torInstance.hpp"


/*
    Basic functions for working with CURL
*/
namespace curl_helper{

    enum errEnum : char {
        OK,
        GENERIC_ERR,
        CURL_ERR,
        TIMEOUT,
        TOR_FAIL,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    /*
        Abstracts the CURL C API as well as providing the ability to route through TOR
    */
    class curlHelper{
    private:
        bool useTor;
        std::string userAgent = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0";
        const int failTimeout = 5;

        TOR::torInstance torInstance;
    
        static size_t curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    public:
        error enableTOR(const int port);
        error downloadFile(const std::string& url, std::vector<char>& curlReadBuffer, int retries=5);
    };

}