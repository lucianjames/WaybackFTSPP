/*
    This program is used to get an index of all pages of a particular domain.
    It fetches the data from the wayback CDX API, then creates an sqlite file which can be loaded by the scraper program.

    Usage: ./get-urls <domain> <output file name> (--no-tor)
*/

//https://web.archive.org/cdx/search?url=*.domain.org/*&fl=mimetype,timestamp,original


#include <iostream>
#include <string>
#include <string.h>

#include "src/urlManager.hpp"



int main(int argc, char** argv){
    if(argc < 3){
        std::cout << "Usage: ./WaybackGetUrls <output filename> <domain1> <domain2> ... <domainN> --tor (optional)\n";
        return 1;
    }

    std::string outfilePath = argv[1];

    url_manager::urlDB udb;
    url_manager::error res = udb.create(outfilePath);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: udb.create(outfilePath): " << res.errmsg << std::endl;
        return 1;
    }

    if(std::string(argv[argc-1]) == "--tor"){
        udb.enableTOR(9051);
        argc--;
    }

    std::vector<std::string> domains;
    for(int i=2; i<argc; i++){
        res = udb.addDomain(argv[i]);
        if(res.errcode != url_manager::errEnum::OK){
            std::cout << "ERR: udb.addDomain(argv[i]): " << res.errmsg << std::endl;
            return 1;
        }
    }

    std::cout << "Complete." << std::endl;
    return 0;
}