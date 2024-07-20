#include "pageScrapeThread.hpp"

pageScraping::error pageScraping::pageScrapeThread::enableTor(const int port){
    curl_helper::error res = this->ch.enableTOR(port);
    if(res.errcode != curl_helper::errEnum::OK){
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
    if(res.errcode != url_manager::errEnum::OK){
        return error{.errcode=UDB_ERR, .errmsg=res.errmsg};
    }
    return error{.errcode=OK, .errmsg=""};
}

/*
    Assumes page.mimetype is compatible
*/
pageScraping::error pageScraping::pageScrapeThread::scrapePage(const url_manager::dbEntry& page){
    curl_helper::parsedPage pageData;
    curl_helper::error getPPRes = this->ch.getParsedPage("https://web.archive.org/web/" + page.timestamp + "/" + page.url, pageData);
    if(getPPRes.errcode != curl_helper::errEnum::OK){
        return error{.errcode=CURLHELPER_ERR, .errmsg=getPPRes.errmsg};
    }

    url_manager::error udbRes = this->udb.setScraped(page.rowID, true);
    if(udbRes.errcode != url_manager::errEnum::OK){
        return error{.errcode=UDB_ERR, .errmsg=udbRes.errmsg};
    }

    manticore::error dbCRes = this->db.connect(); // We dont know if we have connected already or not
    if(dbCRes.errcode != manticore::errEnum::OK){
        return error{.errcode=MANTICORE_ERR, .errmsg=dbCRes.errmsg};
    }

    manticore::error dbIRes = this->db.addPage(page.url, page.timestamp, pageData.title, pageData.text, pageData.raw);
    if(dbIRes.errcode != manticore::errEnum::OK){
        return error{.errcode=MANTICORE_ERR, .errmsg=dbIRes.errmsg};
    }

    return error{.errcode=OK, .errmsg=""};
}


pageScraping::error pageScraping::pageScrapeThread::scrapePages(const std::vector<url_manager::dbEntry>& pageVec){
    for(auto& p : pageVec){
        if(!p.scraped){
            error res = this->scrapePage(p);
            if(res.errcode != OK){
                return res;
            }
        }
    }

    return error{.errcode=OK, .errmsg=""};
}