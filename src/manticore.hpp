#ifndef MANTICORE_HPP
#define MANTICORE_HPP

#include <string>
#include <unordered_map>

#include <json/json.h>

#include "curlHelper.hpp"

namespace manticore{

    enum errEnum : char {
        OK,
        GENERIC_ERR,
        MANTICORE_ERR,
        JSON_ERR,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    class manticoreDB{
    private:
        std::string manticoreServerURL = "127.0.0.1:9308";
        curl_helper::curlHelper ch;
        Json::Reader jsonReader;
        std::string tablename = "waybackFTSPP_pages";
        Json::Value jsonParse(const std::string& json);
        std::string sanitiseStr(const std::string& str);
        error basicQueryExec(const std::string& query);
    public:
        void setTableName(const std::string& name);
        void setServerURL(const std::string& url);
        error connect();
        error addPage(const std::string& url, const std::string& wayback_timestamp, const std::string& title, const std::string& parsed_text_content, const std::string& html);
    };

}

#endif