#include <iostream>

#include "src/manticore.hpp"

int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: ./WaybackScrapePages <ManticoreTableName> <Search query>\n";
        return 1;
    }

    manticore::manticoreDB db;
    db.setServerURL("127.0.0.1:9308"); // This is just default, but making it clear it can be changed.
    db.setTableName(argv[1]);
    db.connect();

    std::vector<manticore::pageEntry> results;
    db.search(argv[2], results);

    for(const auto& r : results){
        std::cout << r.wayback_timestamp << " | " << r.url << std::endl;
    }

    return 0;
}