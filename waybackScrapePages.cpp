#include "src/urlManager.hpp"
#include "src/manticore.hpp"
#include "src/curlHelper.hpp"

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

    for(const auto& e : urlInfoFromSqlite){
        std::cout << "| Mimetype: " << e.mimetype << " | Scraped: " << e.scraped << " | https://web.archive.org/web/" << e.timestamp << "/" << e.url << " |\n";
    }
    std::cout << "Total entries: " << urlInfoFromSqlite.size() << "\n";


    // Download + parse a page to test
    curl_helper::curlHelper ch;
    if(std::string(argv[argc-1]) == "--tor"){
        ch.enableTOR(9051);
    }
    curl_helper::parsedPage pageData;
    ch.getParsedPage("https://web.archive.org/web/" + urlInfoFromSqlite[0].timestamp + "/" + urlInfoFromSqlite[0].url, pageData);

    std::cout << "===========\n";
    std::cout << pageData.title << "\n";
    std::cout << "===========\n";
    std::cout << pageData.text << "\n";
    std::cout << "===========\n";
    std::cout << pageData.raw << "\n";
    std::cout << "===========\n";

    /*
        Indicate that the page has been scraped, so that it doesnt get re-scraped in future
    */
    res = udb.setScraped(urlInfoFromSqlite[0].rowID, true);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: " << res.errmsg << std::endl;
    }

    /*
    manticore::manticoreDB db;
    db.setTableName(argv[1]);
    manticore::error dbcres = db.connect(); // With default server addr 127.0.0.1:9308
    if(dbcres.errcode != manticore::errEnum::OK){
        std::cout << "ERR: db.connect(): " << dbcres.errmsg << std::endl;
    }

    manticore::error dbInsertRes = db.addPage("tes't", "t'e'st", "te''s't", "te''s''t", "t'e's'''t");
    if(dbInsertRes.errcode != manticore::errEnum::OK){
        std::cout << "ERR: db.addPage(): " << dbInsertRes.errmsg << std::endl;
    }
    */
    return 0;
}
