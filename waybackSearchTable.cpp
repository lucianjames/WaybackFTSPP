#include <iostream>
#include <vector>
#include <cxxopts.hpp>
#include "src/manticore.hpp"


int main(int argc, char** argv) {
    cxxopts::Options options("WaybackSearchTable", "Search a manticore database of web pages scraped using WaybackScrapePages");

    options.add_options()
        ("t,table", "Manticore table name", cxxopts::value<std::string>())
        ("q,query", "Search query", cxxopts::value<std::string>())
        ("n,results", "Number of results", cxxopts::value<int>()->default_value("10"))
        ("p,page", "Page number, starts at 0", cxxopts::value<int>()->default_value("0"))
        ("s,server-url", "Manticore database URL (<ip>:<port>)", cxxopts::value<std::string>()->default_value("127.0.0.1:9308"))
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if(!result.count("table") || !result.count("query")){
        std::cout << "Required arguments: -t/--table, -q/--query. Use --help for more information\n";
        return 0;
    }

    // Extract command line arguments
    std::string tableName = result["table"].as<std::string>();
    std::string query = result["query"].as<std::string>();
    int numResults = result["results"].as<int>();
    int page = result["page"].as<int>();
    std::string serverUrl = result["server-url"].as<std::string>();

    manticore::manticoreDB db;
    db.setServerURL(serverUrl);
    db.setTableName(tableName);

    std::vector<manticore::pageEntry> results;
    manticore::error res = db.search(query, results, numResults, page);
    if(res.errcode != manticore::errEnum::OK){
        std::cout << "Search failed: " << res.errmsg << std::endl;
        return 0;
    }

    for (const auto& r : results) {
        std::cout << r.wayback_timestamp << " | " << r.url << std::endl;
    }

    return 0;
}
