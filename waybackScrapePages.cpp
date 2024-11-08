#include <cxxopts.hpp>
#include "src/urlManager.hpp"
#include "src/pageScrapeThread.hpp"

/*
    Function run by each scraper thread
*/
void scrapeThreadFunc(const std::vector<url_manager::dbEntry>& data, const std::string& tableName, const std::string& dbPath, bool useTor, int torPort){
    pageScraping::pageScrapeThread pst;
    if(useTor){
        pst.enableTor(torPort); // should handle this error
    }
    pst.setTableName(tableName);
    pst.udbOpen(dbPath);
    pageScraping::error createTableRes = pst.createTable(); // If table already exists, wont overwrite
    if(createTableRes.errcode != pageScraping::OK){
        std::cout << "ERR: " << createTableRes.errmsg << std::endl;
    }

    pageScraping::error scrapeRes = pst.scrapePages(data);
    std::cout << scrapeRes.errmsg << std::endl;
}


int main(int argc, char** argv){
    /*
        Handle arguments
    */
    cxxopts::Options options("WaybackScrapePages", "Scrape pages from archive.org based on an sqlite db of pages to scrape");
    options.add_options()
        ("t, table", "Manticore table name", cxxopts::value<std::string>())
        ("f,db-file", "File containing URLs to scrape", cxxopts::value<std::string>())
        ("tor", "Route requests through TOR", cxxopts::value<bool>()->default_value("false"))
        ("tor-port", "Port to run TOR proxy on (increments by 1 for each thread)", cxxopts::value<int>()->default_value("9051"))
        ("n-threads", "Number of threads to use", cxxopts::value<int>()->default_value("1"))
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    // Handle printing help msg
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    // Requred options
    if(!result.count("table") || !result.count("db-file")){
        std::cout << "Required arguments: --table, --db-file. Use --help for more information\n";
        return 0;
    }
    // Extract command line arguments
    std::string table = result["table"].as<std::string>();
    std::string dbPath = result["db-file"].as<std::string>();
    bool useTor = result["tor"].as<bool>();
    int torPort = result["tor-port"].as<int>();
    int n_threads = result["n-threads"].as<int>();

    /*
        Open DB file
    */
    url_manager::urlDB udb;
    url_manager::error res = udb.open(dbPath);
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

    /*
        Set up data for multi-threaded scraping
    */
    int chunkSize = urlInfoFromSqlite.size() / n_threads;
    int chunkR = urlInfoFromSqlite.size() % n_threads;
    // This isnt optimal by any means, lots of copying which shouldnt be done.
    // However, 99.99%+ of the execution time of this program will be downloading data so it doesnt really matter.
    // You could do some tricks with iterators to avoid doing this if you really wanted it to be perfect
    std::vector<std::vector<url_manager::dbEntry>> urlInfoChunks;
    urlInfoChunks.resize(n_threads);
    for(int i=0; i<n_threads; i++){
        for(int j=(i==n_threads-1 ? -chunkR : 0); j<chunkSize; j++){ // Negative chunk remainder on last thread to make sure the remaining pieces of data still get processed
            urlInfoChunks[i].push_back(urlInfoFromSqlite[(i*chunkSize)+j]);
        }
    }

    /*
        Start some threads up scraping each element in the urlInfoChunk vector
    */
    std::vector<std::thread> scrapeThreads;
    for(int i=0; i<n_threads; i++){
        scrapeThreads.emplace_back(scrapeThreadFunc, urlInfoChunks[i], table, dbPath, useTor, torPort);
        torPort++; // Increment so we get a fresh instance of TOR (thus a fresh IP) for every thread
    }
    // Wait for all threads to finish
    for (auto& t : scrapeThreads) {
        t.join();
    }

    return 0;
}

