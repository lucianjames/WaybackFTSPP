#ifndef CURLHELPER_HPP
#define CURLHELPER_HPP

#include <string>
#include <vector>

#include <curl/curl.h>
#include <libxml/HTMLparser.h>

#include "torInstance.hpp"
#include "URLUtils.hpp"


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
        LIBXML_FAIL,
        LIBXML_NULLDOC,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    struct parsedPage{
        std::string title;
        std::string text;
        std::string raw;
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
        static size_t curlWriteCallback_bin(void* contents, size_t size, size_t nmemb, void* userp);
        static size_t curlWriteCallback_str(char* contents, size_t size, size_t nmemb, void* userp);
        error htmlTreeGetTitle(xmlDoc* doc, xmlNodePtr node, std::string& titleBuff);
        error htmlTreeGetText(xmlNodePtr node, std::string& textBuff);
        error htmlParse(const std::string& pageSrc, htmlParserCtxtPtr& parser);
        error getTitleFromPage(const std::string& pageSrc, std::string& titleOut);
        error getTextFromPage(const std::string& pageSrc, std::string& textOut);
    public:
        error enableTOR(const int port);
        error downloadFile(const std::string& url, std::vector<char>& curlReadBuffer, int retries=5);
        error manticoreQuery(const std::string& serverAddr, const std::string& query, std::string& readBuffer);
        error getParsedPage(const std::string& url, parsedPage& out);
    };

}

#endif