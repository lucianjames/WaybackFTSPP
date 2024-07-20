#include "src/urlManager.hpp"
#include "src/pageScrapeThread.hpp"


int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: ./WaybackScrapePages <ManticoreTableName> <URLs sqlite file> --tor (optional)\n";
        return 1;
    }

    //const int n_threads = 8; // TODO make adjustable via cmdline

    /*
        Open DB file
    */
    url_manager::urlDB udb;
    url_manager::error res = udb.open(argv[2]);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: udb.open(dbPath): " << res.errmsg << std::endl;
        return 1;
    }

    /*
        Get information from DB file
    */
    std::vector<url_manager::dbEntry> urlInfoFromSqlite;
    std::vector<std::string> allowed_mimetypes = {
        url_manager::mimetypes::TEXT_HTML,
        url_manager::mimetypes::TEXT_PLAIN,
        url_manager::mimetypes::TEXT_XML,
    };
    res = udb.getData(urlInfoFromSqlite, true, allowed_mimetypes);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: " << res.errmsg << std::endl;
    }

    pageScraping::pageScrapeThread pst;
    pst.enableTor(9051);
    pst.setTableName(argv[1]); // This is mostly required
    pst.udbOpen(argv[2]); // This is required

    for(auto& u : urlInfoFromSqlite){
        if(u.scraped == false){
            pst.scrapePage(u);
        }
    }
    
    return 0;
}
