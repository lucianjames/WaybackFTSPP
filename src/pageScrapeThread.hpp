#ifndef PAGESCRAPETHREAD_HPP
#define PAGESCRAPETHREAD_HPP

#include <string>

#include "manticore.hpp"
#include "curlHelper.hpp"
#include "urlManager.hpp" // Used to set URLs as scraped == true. Getting data from DB not done by this class, scraping only


namespace pageScraping{

    enum errEnum : char {
        OK,
        GENERIC_ERR,
        CURLHELPER_ERR,
        MANTICORE_ERR,
        UDB_ERR,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    class pageScrapeThread {
    private:
        curl_helper::curlHelper ch;
        manticore::manticoreDB db;
        url_manager::urlDB udb;
    public:
        error enableTor(const int port); // To access func of this->ch
        void setServerURL(const std::string& url); // To access func of this->db
        void setTableName(const std::string& name); // ^
        error udbOpen(const std::string& dbPath); // To access func of this->udb
        error scrapePage(const url_manager::dbEntry& page);
    };

}

#endif