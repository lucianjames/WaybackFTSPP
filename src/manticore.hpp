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

    struct pageEntry {
        unsigned long long ID;
        std::string url;
        std::string wayback_timestamp;
        std::string title;
        std::string parsed_text_content;
        std::string html;
        std::string highlighted_content;
    };

    class manticoreDB{
    private:
        std::string manticoreServerURL = "127.0.0.1:9308";
        curl_helper::curlHelper ch;
        Json::Reader jsonReader;
        std::string tablename = "waybackFTSPP_pages";
        Json::Value jsonParse(const std::string& json);
        std::string sanitiseStr(const std::string& str);
        std::string unSanitiseStr(const std::string& str);
        error basicQueryExec(const std::string& query);
        error basicQueryExec(const std::string& query, Json::Value& manticoreRes_parsed_out);
    public:
        void setTableName(const std::string& name);
        void setServerURL(const std::string& url);
        error setup();
        error addPage(const std::string& url, const std::string& wayback_timestamp, const std::string& title, const std::string& parsed_text_content, const std::string& html);
        error search(const std::string& query, std::vector<pageEntry>& results_out, const int n_results_pp, const int page);
        error getTables(std::vector<std::string>& out);
    };

}

#endif