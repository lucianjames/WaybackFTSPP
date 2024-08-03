#include <iostream>
#include <string>
#include <cxxopts.hpp>
#include "src/urlManager.hpp"


int main(int argc, char** argv){
    /*
        Handle arguments
    */
    cxxopts::Options options("WaybackGetUrls", "Get pages archived on archive.org for a given domain");
    options.add_options()
        ("o,output_file", "Sqlite3 DB name", cxxopts::value<std::string>())
        ("d,domain", "Search query", cxxopts::value<std::string>())
        ("tor", "Route requests through TOR", cxxopts::value<bool>()->default_value("false"))
        ("tor-port", "Port to run TOR proxy on", cxxopts::value<int>()->default_value("9051"))
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    // Handle printing help msg
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    // Requred options
    if(!result.count("output_file") || !result.count("domain")){
        std::cout << "Required arguments: --output_file, --domain. Use --help for more information\n";
        return 0;
    }
    // Extract command line arguments
    std::string outFile = result["output_file"].as<std::string>();
    std::string domain = result["domain"].as<std::string>();
    bool useTor = result["tor"].as<bool>();
    int torPort = result["tor-port"].as<int>();

    /*
        Create DB file
    */
    url_manager::urlDB udb;
    url_manager::error res = udb.create(outFile);
    if(res.errcode != url_manager::errEnum::OK){
        // If the db file already exists, lets simply open it so we can add the new domains
        if(res.errcode == url_manager::errEnum::FILE_EXISTS){
            udb.open(outFile);
        }else{
            std::cout << "ERR: udb.create(outfilePath): " << res.errmsg << std::endl;
            return 1;
        }
    }

    /*
        Enable TOR (if requested)
    */
    if(useTor){
        udb.enableTOR(torPort);
    }

    /*
        Add domain to DB. This performs everything from getting CDX data to adding it to the sqlite file
    */
    res = udb.addDomain(domain);
    if(res.errcode != url_manager::errEnum::OK){
        std::cout << "ERR: udb.addDomain(argv[i]): " << res.errmsg << std::endl;
        return 1;
    }

    std::cout << "Complete." << std::endl;
    return 0;
}