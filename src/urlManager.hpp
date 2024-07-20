#ifndef URLMANAGER_HPP
#define URLMANAGER_HPP


#include <string>
#include <filesystem>
#include <sstream>
#include <algorithm>

#include <sqlite3.h>

#include "curlHelper.hpp"

/*
    The url manager is used to create, fill, and read files containing wayback archive data to scrape.
*/
namespace url_manager{

    enum errEnum : char {
        OK,
        GENERIC_ERR,
        SQLITE_ERR, // Maybe bad name, can be confused with SQLITE_ERROR
        BAD_ARG,
        NOT_INIT,
        API_BADDATA,
        CURL_FAIL,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    struct dbEntry {
        int rowID;
        std::string url;
        std::string timestamp;
        std::string mimetype;
        int scraped;
    };

    /*
        Used within the urlDB class for quick basic error handling of sqlite calls
    */
    #define SQLITE_CALL(func, db) \
    if(func != SQLITE_OK) { \
        std::cout << "SQLite error: " << sqlite3_errmsg(db); \
    } \

    namespace mimetypes {
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
    
    /*
        urlDB provides easy functions for creating and reading SQLITE database files containing wayback page URLs
    */
    class urlDB {
    private:
        sqlite3* db = NULL;
        curl_helper::curlHelper ch;
    public:
        ~urlDB();
        error create(const std::string& dbPath);
        error open(const std::string& dbPath);
        error addDomain(const std::string& domain);
        error enableTOR(const int port);
        error getData(std::vector<dbEntry>& out, bool unscraped_only, const std::vector<std::string>& allowed_mimetypes);
        error setScraped(const int ID, const bool val);
    };

}

#endif