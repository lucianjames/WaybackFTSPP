#include "pageScrapeThread.hpp"

pageScraping::error pageScraping::pageScrapeThread::enableTor(const int port){
    curl_helper::error res = this->ch.enableTOR(port);
    if(res.errcode != curl_helper::OK){
        return error{.errcode=CURLHELPER_ERR, .errmsg=res.errmsg};
    }
    return error{.errcode=OK, .errmsg=""};
}

void pageScraping::pageScrapeThread::setServerURL(const std::string& url){
    this->db.setServerURL(url);
}

void pageScraping::pageScrapeThread::setTableName(const std::string& name){
    this->db.setTableName(name);
}

pageScraping::error pageScraping::pageScrapeThread::udbOpen(const std::string& dbPath){
    url_manager::error res = this->udb.open(dbPath);
    if(res.errcode != url_manager::OK){
        return error{.errcode=UDB_ERR, .errmsg=res.errmsg};
    }
    return error{.errcode=OK, .errmsg=""};
}

/*
    DO THE ERROR HANDLING

    Assumes page.mimetype is compatible
*/
pageScraping::error pageScraping::pageScrapeThread::scrapePage(const url_manager::dbEntry& page){
    curl_helper::parsedPage pageData;
    curl_helper::error getPPRes = this->ch.getParsedPage("https://web.archive.org/web/" + page.timestamp + "/" + page.url, pageData);

    url_manager::error udbRes = this->udb.setScraped(page.rowID, true);

    manticore::error dbCRes = this->db.connect(); // We dont know if we have connected already or not
    manticore::error dbIRes = this->db.addPage(page.url, page.timestamp, pageData.title, pageData.text, pageData.raw);

    return error{.errcode=OK, .errmsg=""};
}