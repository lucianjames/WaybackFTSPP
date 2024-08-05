#include <fstream>

#include "src/manticore.hpp"

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <json/json.h>

class SearchHandler : public Pistache::Http::Handler{
private:
    void serveStaticFile(const std::string& fileName, Pistache::Http::ResponseWriter response);
    std::string performWaybackSearch(const std::string& query, const std::string& tableName);
public:
    HTTP_PROTOTYPE(SearchHandler)
    void onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response) override;
};

void SearchHandler::serveStaticFile(const std::string& fileName, Pistache::Http::ResponseWriter response){
    std::ifstream file(fileName);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.send(Pistache::Http::Code::Ok, buffer.str());
    } else {
        response.send(Pistache::Http::Code::Internal_Server_Error, "Error opening file.");
    }
}

std::string SearchHandler::performWaybackSearch(const std::string& query, const std::string& tableName){
    int maxNumResults = 20;
    int page = 0;
    std::string serverUrl = "127.0.0.1:9308";
    
    manticore::manticoreDB db;
    db.setServerURL(serverUrl);
    db.setTableName(tableName);

    std::vector<manticore::pageEntry> results;
    manticore::error res = db.search(query, results, maxNumResults, page);
    if(res.errcode != manticore::errEnum::OK){
        return "Search failed: " + res.errmsg;
    }

    std::string resultsPage = "<!DOCTYPE html><html><head><title>Search Results</title></head><body>";
    for(const auto& r : results){

        resultsPage += "<div><h2><a href=\"https://web.archive.org/web/" + r.wayback_timestamp + "/" + r.url + "\">" 
        + ((r.title.length() == 0)? "NO_TITLE" : r.title)
        + "</a></h2>";

        resultsPage += "<p> blah blah </p></div>";
    }
    resultsPage += "</body></html>";

    return resultsPage;
}

void SearchHandler::onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response){
    auto method = request.method();
        auto path = request.resource();

        /*
            Handle GET requests
        */
        if (method == Pistache::Http::Method::Get) {
            if (path == "/") {
                serveStaticFile("webPages/search_index.html", std::move(response));
            } else {
                response.send(Pistache::Http::Code::Not_Found, "404");
            }
        } 
        /*
            Handle POST requests
        */
        else if (method == Pistache::Http::Method::Post) {
            if (path == "/search") {
                // Parse query + table name json params
                std::string json = request.body();
                Json::Reader jsonReader;
                Json::Value root;
                if(!jsonReader.parse(json, root)){
                    response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to parse JSON" + json);
                    throw std::runtime_error("Failed to parse JSON: " + json);
                }
                // Perform search
                std::string resultPage = this->performWaybackSearch(root["q"].asString(), root["tn"].asString());
                response.send(Pistache::Http::Code::Ok, resultPage);
            } else {
                response.send(Pistache::Http::Code::Not_Found, "404");
            }
        }
        /*
            Handle unsupported requests
        */
        else {
            response.send(Pistache::Http::Code::Method_Not_Allowed, "405");
        }
}

#define portn 8088
int main(){
    std::cout << "http://127.0.0.1:" << portn << std::endl;

    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(portn));
    auto opts = Pistache::Http::Endpoint::options()
                            .threads(1)
                            .flags(Pistache::Tcp::Options::ReusePort);
    Pistache::Http::Endpoint server(addr);

    server.init(opts);
    server.setHandler(Pistache::Http::make_handler<SearchHandler>());
    server.serve();

    return 0;
}