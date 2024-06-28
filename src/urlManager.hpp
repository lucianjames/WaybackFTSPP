#include <string>
#include <filesystem>
#include <sstream>

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
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    struct dbEntry {
        int ID;
        std::string url;
        std::string timestamp;
        std::string mimetype;
        int scraped;
    };

    
    /*
        urlDB provides easy functions for creating and reading SQLITE database files containing wayback page URLs
    */
    class urlDB {
    private:
        sqlite3* db = NULL;
        const std::string sql_create_table = "CREATE TABLE URLS(" \
                                                "URL TEXT NOT NULL, " \
                                                "TIMESTAMP TEXT NOT NULL, " \
                                                "MIMETYPE TEXT NOT NULL, " \
                                                "SCRAPED INT NOT NULL);"; // Used by scraper program to mark
        curl_helper::curlHelper ch;
    public:
        ~urlDB();
        error create(const std::string& dbPath);
        error addDomain(const std::string& domain);
    };

}