#include "manticore.hpp"

Json::Value manticore::manticoreDB::jsonParse(const std::string& json){
    Json::Value root;
    if(!this->jsonReader.parse(json, root)){
        throw std::runtime_error("Failed to parse JSON: " + json);
    }
    return root;
}

std::string manticore::manticoreDB::sanitiseStr(const std::string& str) {
    std::unordered_map<char, std::string> replacements = {
        {'\'', "{{APOSTROPHE}}"},
        {'"', "{{QUOTE}}"},
        {';', "{{SEMICOLON}}"},
        {'\\', "{{BACKSLASH}}"},
        {'*', "{{ASTERISK}}"},
        {'=', "{{EQUAL}}"},
        {'(', "{{LPAREN}}"},
        {')', "{{RPAREN}}"},
        {'<', "{{LT}}"},
        {'>', "{{GT}}"},
        {'!', "{{EXCLAMATION}}"},
        {'/', "{{SLASH}}"},
        {'-', "{{DASH}}"}
    };

    std::string res;
    res.reserve(str.size()); // Only perfect if no replacements need to be inserted

    for (const char& ch : str) {
        auto it = replacements.find(ch);
        if (it != replacements.end()) {
            res += it->second;
        } else {
            res += ch;
        }
    }

    return res;
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
    this->tablename = name;
}

manticore::error manticore::manticoreDB::connect(){
    manticore::error createTableRes = this->basicQueryExec("CREATE TABLE IF NOT EXISTS " + this->tablename + "("
                                                                             "id bigint,"
                                                                             "url text attribute,"
                                                                             "wayback_timestamp text attribute,"
                                                                             "title text attribute,"
                                                                             "parsed_text_content text,"
                                                                             "html text attribute engine='columnar')");

    return createTableRes;
}


manticore::error manticore::manticoreDB::addPage(const std::string& url, const std::string& wayback_timestamp, const std::string& title, const std::string& parsed_text_content, const std::string& html){
    std::string url_sanitised = this->sanitiseStr(url);
    std::string wayback_timestamp_sanisised = this->sanitiseStr(wayback_timestamp);
    std::string title_sanitised = this->sanitiseStr(title);
    std::string parsed_text_sanitised = this->sanitiseStr(parsed_text_content);
    std::string html_sanitised = this->sanitiseStr(html);

    // Todo ensure no page is inserted twice?

    return this->basicQueryExec("INSERT INTO " + this->tablename + " (url, wayback_timestamp, title, parsed_text_content, html) VALUES ('"
                                                            + url_sanitised + "', '" 
                                                            + wayback_timestamp_sanisised + "', '" 
                                                            + title_sanitised + "', '" 
                                                            + parsed_text_sanitised + "', '" 
                                                            + html_sanitised + "')");
}