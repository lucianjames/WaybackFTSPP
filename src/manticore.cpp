#include "manticore.hpp"

Json::Value manticore::manticoreDB::jsonParse(const std::string& json){
    Json::Value root;
    if(!this->jsonReader.parse(json, root)){
        throw std::runtime_error("Failed to parse JSON: " + json);
    }
    return root;
}

manticore::error manticore::manticoreDB::basicQueryExec(const std::string& query){
    std::string manticoreRes;
    this->ch.manticoreQuery(this->manticoreServerURL, query, manticoreRes);
    Json::Value root;
    if(!this->jsonReader.parse(manticoreRes, root)){
        return error{.errcode=JSON_ERR, .errmsg="basicQueryExec: Failed to parse JSON"};
    }
    if(root[0]["error"].asString() != ""){
        return error{.errcode=MANTICORE_ERR, .errmsg="basicQueryExec: Manticore error: " + root[0]["error"].asString() + " (Query: " + query + ")"};
    }
    return error{.errcode=OK, .errmsg=""};
}

void manticore::manticoreDB::setTableName(const std::string& name){

}

manticore::error manticore::manticoreDB::connect(){
    manticore::error createTableRes = this->basicQueryExec("CREATE TABLE IF NOT EXISTS " + this->tablename + "("
                                                                             "id bigint,"
                                                                             "url bigint,"
                                                                             "wayback_timestamp bigint,"
                                                                             "title text attribute,"
                                                                             "parsed_text_content text,"
                                                                             "html text attribute engine='columnar')");

    return createTableRes;
}