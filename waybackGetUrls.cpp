#include <iostream>
#include <string>
#include <cxxopts.hpp>
#include "src/urlManager.hpp"
#include "src/manticore.hpp"


int main(int argc, char** argv){
    /*
        Handle arguments
    */
    cxxopts::Options options("WaybackGetUrls", "Get pages archived on archive.org for a given domain");
    options.add_options()
        ("t,url-table-name", "Manticore URL DB table name", cxxopts::value<std::string>())
        ("d,domain", "Domain pattern to get URLs for", cxxopts::value<std::string>())
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
    if(!result.count("url-table-name") || !result.count("domain")){
        std::cout << "Required arguments: --url-table-name, --domain. Use --help for more information\n";
        return 0;
    }
    // Extract command line arguments
    std::string tableName = result["url-table-name"].as<std::string>();
    std::string domain = result["domain"].as<std::string>();
    bool useTor = result["tor"].as<bool>();
    int torPort = result["tor-port"].as<int>();

    /*
        Set up manticore connection
    */
    manticore::manticoreDB db;
    db.setURLDBTableName(tableName);
    manticore::error ct_res = db.URLDB_createTable();
    if(ct_res.errcode != manticore::errEnum::OK){
        std::cout << "ERR: db.URLDB_createTable()" << ct_res.errmsg << std::endl;
        return 1;
    }
    if(useTor){
        db.URLDB_enableTOR(torPort);
    }

    /*
        Add the domain to the table
    */
    manticore::error ad_res = db.URLDB_addDomain(domain);
    if(ad_res.errcode != manticore::errEnum::OK){
        std::cout << "ERR: db.URLDB_addDomain(domain): " << ad_res.errmsg.substr(0, 1024) << std::endl; // substr because its a very very long sql query
        return 1;
    }
    std::cout << "Complete." << std::endl;

    return 0;
}