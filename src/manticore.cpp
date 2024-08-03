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
        } else if(std::isprint(ch)){ // Maybe turn non-printable chars into some kind of hex encoding?
            res += ch;
        }
    }

    return res;
}


std::string manticore::manticoreDB::unSanitiseStr(const std::string& str) {
    std::unordered_map<std::string, std::string> replacements = {
        {"{{APOSTROPHE}}", "\'"},
        {"{{QUOTE}}", "\""},
        {"{{SEMICOLON}}", ";"},
        {"{{BACKSLASH}}", "\\"},
        {"{{ASTERISK}}", "*"},
        {"{{EQUAL}}", "="},
        {"{{LPAREN}}", "("},
        {"{{RPAREN}}", ")"},
        {"{{LT}}", "<"},
        {"{{GT}}", ">"},
        {"{{EXCLAMATION}}", "!"},
        {"{{SLASH}}", "/"},
        {"{{DASH}}", "-"}
    };

    std::string mod_str;

    for(size_t i=0; i<str.size(); i++){
        if(str[i] == '{'){
            size_t tagLen = str.find_first_of("}}", i) - i + 2;
            std::string tag = str.substr(i, tagLen);
            auto r_it = replacements.find(tag);
            if(r_it != replacements.end()){
                mod_str += r_it->second;
                i += r_it->first.length()-1;
            }else{
                mod_str += str[i];
            }
        }else{
            mod_str += str[i];
        }

    }

    return mod_str;
}


manticore::error manticore::manticoreDB::basicQueryExec(const std::string& query){
    Json::Value root_unused;
    return this->basicQueryExec(query, root_unused);
}


manticore::error manticore::manticoreDB::basicQueryExec(const std::string& query, Json::Value& manticoreRes_parsed_out){
    std::string manticoreRes;
    this->ch.manticoreQuery(this->manticoreServerURL, query, manticoreRes);
    Json::Value root;
    if(!this->jsonReader.parse(manticoreRes, root)){
        return error{.errcode=JSON_ERR, .errmsg="basicQueryExec: Failed to parse JSON"};
    }
    if(root[0]["error"].asString() != ""){
        return error{.errcode=MANTICORE_ERR, .errmsg="basicQueryExec: Manticore error: " + root[0]["error"].asString() + " (Query: " + query + ")"};
    }
    manticoreRes_parsed_out = root;
    return error{.errcode=OK, .errmsg=""};
}


void manticore::manticoreDB::setTableName(const std::string& name){
    this->tablename = name;
}


void manticore::manticoreDB::setServerURL(const std::string& URL){
    this->manticoreServerURL = URL;
}


manticore::error manticore::manticoreDB::setup(){
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


manticore::error manticore::manticoreDB::search(const std::string& query, std::vector<pageEntry>& results_out, const int n_results_pp, const int page){
    Json::Value json_res;
    std::string sanitised_query = this->sanitiseStr(query);
    error bqe_res = this->basicQueryExec(
                                        "SELECT * FROM " + this->tablename + " WHERE MATCH(\'" + sanitised_query + "\') LIMIT " + std::to_string(n_results_pp) + " OFFSET " + std::to_string(page*n_results_pp) + ";", 
                                        json_res);
    if(bqe_res.errcode != OK){
        return error{.errcode=MANTICORE_ERR, .errmsg=bqe_res.errmsg};
    }

    // Parse json result shit
    std::cout << "Parsing..." << std::endl;
    for(const auto& result : json_res[0]["data"]){
        results_out.push_back(
            pageEntry{
                .ID = result["id"].asUInt64(),
                .url = this->unSanitiseStr(result["url"].asString()),
                .wayback_timestamp = this->unSanitiseStr(result["wayback_timestamp"].asString()),
                .title = this->unSanitiseStr(result["title"].asString()),
                .parsed_text_content = this->unSanitiseStr(result["parsed_text_content"].asString()),
                .html = this->unSanitiseStr(result["html"].asString()), // This can be very time consuming... Probably best not do do this until the raw HTML is really required.
            }
        );
    }

    return error{.errcode=OK, .errmsg=""};
}