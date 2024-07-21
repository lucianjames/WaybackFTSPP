#include "src/urlManager.hpp"
#include "src/pageScrapeThread.hpp"


void scrapeThreadFunc(const std::vector<url_manager::dbEntry>& data, const std::string& tableName, const std::string& dbPath, int enableTorPort){
    pageScraping::pageScrapeThread pst;
    if(enableTorPort != -1){
        pst.enableTor(enableTorPort);
    }
    pst.setTableName(tableName);
    pst.udbOpen(dbPath);

    pageScraping::error scrapeRes = pst.scrapePages(data);
    std::cout << scrapeRes.errmsg << std::endl;
}


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
    if(std::string(argv[argc-1]) == "--tor"){
        pst.enableTor(9051);
    }    
    pst.setTableName(argv[1]); // This is mostly required
    pst.udbOpen(argv[2]); // This is required

    //pageScraping::error scrapeRes = pst.scrapePages(urlInfoFromSqlite);
    //std::cout << scrapeRes.errmsg << std::endl;

    const int n_threads = 8;
    int chunkSize = urlInfoFromSqlite.size() / n_threads;
    int chunkR = urlInfoFromSqlite.size() % n_threads;

    std::cout << urlInfoFromSqlite.size() << std::endl;
    std::cout << chunkSize << std::endl;
    std::cout << chunkR << std::endl;

    /*
        This isnt optimal by any means, lots of copying which shouldnt be done.
        However, 99.99% of the execution time of this program will be downloading data so it doesnt really matter.
        You could do some tricks with iterators to avoid doing this if you really wanted it to be perfect
    */
    std::vector<std::vector<url_manager::dbEntry>> urlInfoChunks;
    urlInfoChunks.resize(n_threads);
    for(int i=0; i<n_threads; i++){
        for(int j=(i==n_threads-1 ? -chunkR : 0); j<chunkSize; j++){
            urlInfoChunks[i].push_back(urlInfoFromSqlite[(i*chunkSize)+j]);
        }
    }

    for(auto& chnk : urlInfoChunks){
        std::cout << chnk.size() << std::endl;
    }

    std::vector<std::thread> scrapeThreads;

    for(int i=0; i<n_threads; i++){
        scrapeThreads.emplace_back(scrapeThreadFunc, urlInfoChunks[i], argv[1], argv[2], 9051); // Hardcode bad this is just test
    }

    for (auto& t : scrapeThreads) {
        t.join();
    }

    return 0;
}

