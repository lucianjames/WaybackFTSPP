#ifndef MANTICORE_HPP
#define MANTICORE_HPP

#include <string>
#include <unordered_map>

#include <json/json.h>

#include "curlHelper.hpp"
#include "urlManager.hpp"

namespace manticore{

    enum errEnum : char {
        OK,
        GENERIC_ERR,
        MANTICORE_ERR,
        JSON_ERR,
        CURL_FAIL,
        API_BADDATA,
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

    struct URLDBEntry {
        int rowID;
        std::string url;
        std::string timestamp;
        std::string mimetype;
        int scraped;
    };

    namespace URLDBMimetypes {
        const std::string TEXT_HTML = "text/html";
        const std::string TEXT_PLAIN = "text/plain";
        const std::string TEXT_XML = "text/xml";
        const std::string TEXT_JAVASCRIPT = "text/javascript";
        const std::string APPLICATION_PDF = "application/pdf";
        const std::string APPLICATION_JAVASCRIPT = "application/javascript";
        const std::string WARC_REVISIT = "warc/revisit";
        const std::string IMAGE_PNG = "image/png";
        const std::string IMAGE_JPG = "image/jpg";
    }

    class manticoreDB{
    private:
        std::string manticoreServerURL = "127.0.0.1:9308";
        curl_helper::curlHelper ch;
        curl_helper::curlHelper ch_cdxAPI; // Separate curl instance for CDX api, in case TOR is enabled.
        Json::Reader jsonReader;
        std::string tablename = "pages_waybackFTSPP";
        std::string URLDB_tablename = "urldb_waybackFTSPP";
        Json::Value jsonParse(const std::string& json);
        std::string sanitiseStr(const std::string& str);
        std::string unSanitiseStr(const std::string& str);
        error basicQueryExec(const std::string& query);
        error basicQueryExec(const std::string& query, Json::Value& manticoreRes_parsed_out);
    public:
        void setTableName(const std::string& name);
        void setServerURL(const std::string& url);
        error createTable();
        error addPage(const std::string& url, const std::string& wayback_timestamp, const std::string& title, const std::string& parsed_text_content, const std::string& html);
        error search(const std::string& query, std::vector<pageEntry>& results_out, const int n_results_pp, const int page);
        error getTables(std::vector<std::string>& out);

        // Port urlManager from sqlite to manticore
        void setURLDBTableName(const std::string& name);
        error URLDB_createTable();
        error URLDB_addDomain(const std::string& domain);
        error URLDB_enableTOR(const int port);
        error URLDB_getData(std::vector<URLDBEntry>& out, bool unscraped_only, const std::vector<std::string>& allowed_mimetypes = {});
        error URLDB_setScraped(const int ID, const bool val);
    };

}

#endif