/*
    This program is used to get an index of all pages of a particular domain.
    It fetches the data from the wayback CDX API, then creates an sqlite file which can be loaded by the scraper program.

    Usage: ./get-urls <domain> <output file name> (--no-tor)
*/

//https://web.archive.org/cdx/search?url=*.domain.org/*&fl=mimetype,timestamp,original


#include <iostream>
#include <string>

#include "src/urlManager.hpp"



int main(int argc, char** argv){
    /*
        Get cmdline args
    */
    if(argc < 3){
        std::cout << "Usage: ./WaybackGetUrls <output filename> <domain1> <domain2> ... <domainN>\n";
        return 1;
    }
    std::string outfilePath = argv[1];
    std::string domain = argv[2];
    // TOOOODOOOO HANDLE N DOMAINS INSTEAD OF JUST 1 !!!!!!!

    url_manager::urlDB udb;

    url_manager::error res = udb.create(outfilePath);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: udb.create(outfilePath): " << res.errmsg << std::endl;
    }

    res = udb.addDomain(domain);

    return 0;
}