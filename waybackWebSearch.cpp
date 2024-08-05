#include <fstream>

#include "src/manticore.hpp"

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include <json/json.h>

class SearchHandler : public Pistache::Http::Handler{
private:
    void serveStaticFile(const std::string& fileName, Pistache::Http::ResponseWriter response);
    std::string performWaybackSearch(const std::string& query, const std::string& tableName, const int pageNum);
public:
    HTTP_PROTOTYPE(SearchHandler)
    void onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response) override;
};

void SearchHandler::serveStaticFile(const std::string& fileName, Pistache::Http::ResponseWriter response){
    std::ifstream file(fileName);
    if(file.is_open()){
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.send(Pistache::Http::Code::Ok, buffer.str());
    }else{
        response.send(Pistache::Http::Code::Internal_Server_Error, "Error opening file.");
    }
}

std::string SearchHandler::performWaybackSearch(const std::string& query, const std::string& tableName, const int pageNum){
    // Setup
    int maxNumResults = 20;
    std::string serverUrl = "127.0.0.1:9308";
    manticore::manticoreDB db;
    db.setServerURL(serverUrl);
    db.setTableName(tableName);

    /*
        Perform search via manticore
    */
    std::vector<manticore::pageEntry> results;
    manticore::error res = db.search(query, results, maxNumResults, pageNum);
    if(res.errcode != manticore::errEnum::OK){
        return "Search failed: " + res.errmsg;
    }

    /*
        Generate results cards
    */
    std::string resultCardsHTML;
    for(const auto& r : results){
       resultCardsHTML += "<div class=\"result-card\">"
                        "<h2><a href=\"https://web.archive.org/web/" + r.wayback_timestamp + "/" + r.url + "\">" + ((r.title.length() == 0)? r.url : r.title)
                     + "</a></h2>";
        resultCardsHTML += "<h4>" + r.wayback_timestamp + "</h4>";
        resultCardsHTML += "<p>" + r.highlighted_content + "</p>";
        resultCardsHTML += "</div>";
    }

    /*
        The clientside code will insert the result cards to where they need to go
    */
    return resultCardsHTML;
}

void SearchHandler::onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response){
    auto method = request.method();
        auto path = request.resource();

        /*
            Handle GET requests
        */
        if (method == Pistache::Http::Method::Get){
            if(path == "/"){
                serveStaticFile("webPages/search_index.html", std::move(response));
            }else if(path == "/tables"){
                std::vector<std::string> tables;
                manticore::manticoreDB db;
                db.getTables(tables);

                std::string tableJson = "{\"tables\": [";
                for(int i=0; i<tables.size(); i++){
                    tableJson += "\"" + tables[i] + "\"";
                    if(i != tables.size()-1){
                        tableJson += ", ";
                    }
                }
                tableJson += "]}";

                response.send(Pistache::Http::Code::Ok, tableJson);
            }else{
                serveStaticFile("webPages/" + path, std::move(response));
            }
        } 
        /*
            Handle POST requests
        */
        else if (method == Pistache::Http::Method::Post){
            if(path == "/search"){
                // Parse query + table name json params
                std::string json = request.body();
                Json::Reader jsonReader;
                Json::Value root;
                if(!jsonReader.parse(json, root)){
                    response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to parse JSON" + json);
                    throw std::runtime_error("Failed to parse JSON: " + json);
                }
                // Perform search
                std::string resultPage = this->performWaybackSearch(root["q"].asString(), root["tn"].asString(), root["p"].asInt());
                response.send(Pistache::Http::Code::Ok, resultPage);
            } else{
                response.send(Pistache::Http::Code::Not_Found, "404");
            }
        }
        /*
            Handle unsupported requests
        */
        else{
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