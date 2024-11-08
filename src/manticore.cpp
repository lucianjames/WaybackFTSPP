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
    }; // Not sure if all of these are required to filter out or not, just doing them all to be extra safe

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
    this->tablename = "pages_" + name; // Prefix allows easily filtering page and urldbs when using SHOW TABLES
}


void manticore::manticoreDB::setServerURL(const std::string& URL){
    this->manticoreServerURL = URL;
}


manticore::error manticore::manticoreDB::createTable(){
    error createTableRes = this->basicQueryExec("CREATE TABLE IF NOT EXISTS " + this->tablename + "("
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
                                        "SELECT *, HIGHLIGHT({around=15, limit=512}) FROM " + this->tablename + " WHERE MATCH(\'" + sanitised_query + "\') LIMIT " + std::to_string(n_results_pp) + " OFFSET " + std::to_string(page*n_results_pp) + ";", 
                                        json_res);
    if(bqe_res.errcode != OK){
        return error{.errcode=MANTICORE_ERR, .errmsg=bqe_res.errmsg};
    }

    // Parse json result shit
    for(const auto& result : json_res[0]["data"]){
        results_out.push_back(
            pageEntry{
                .ID = result["id"].asUInt64(),
                .url = this->unSanitiseStr(result["url"].asString()),
                .wayback_timestamp = this->unSanitiseStr(result["wayback_timestamp"].asString()),
                .title = this->unSanitiseStr(result["title"].asString()),
                .parsed_text_content = this->unSanitiseStr(result["parsed_text_content"].asString()),
                .html = this->unSanitiseStr(result["html"].asString()), // This can be very time consuming... Probably best not do do this until the raw HTML is really required.
                .highlighted_content = this->unSanitiseStr(result["highlight({around=15, limit=512})"].asString()),
            }
        );
    }

    return error{.errcode=OK, .errmsg=""};
}


manticore::error manticore::manticoreDB::getTables(std::vector<std::string>& out){
    Json::Value json_res;
    std::string query = "SELECT table_name FROM information_schema.tables;";
    error bqe_res = this->basicQueryExec(query, json_res);
    if(bqe_res.errcode != OK){
        return error{.errcode=MANTICORE_ERR, .errmsg=bqe_res.errmsg};
    }

    for(const auto& table : json_res[0]["data"]){
        out.push_back(table["TABLE_NAME"].asString());
    }

    return error{.errcode=OK, .errmsg=""};
}


void manticore::manticoreDB::setURLDBTableName(const std::string& name){
    this->URLDB_tablename = "urldb_" + name;
}


manticore::error manticore::manticoreDB::URLDB_createTable(){
    error createTableRes = this->basicQueryExec("CREATE TABLE IF NOT EXISTS " + this->URLDB_tablename + " ("
                                                                             "id bigint,"
                                                                             "url text attribute,"
                                                                             "wayback_timestamp text attribute,"
                                                                             "mimetype text attribute,"
                                                                             "scraped int)");

    return createTableRes;
}


manticore::error manticore::manticoreDB::URLDB_addDomain(const std::string& domain){
    /*
        Get page list from API
    */
    std::vector<char> buff;
    std::cout << "https://web.archive.org/cdx/search?url=" + domain + "&fl=mimetype,timestamp,original\n";
    curl_helper::error downloadFileRes = this->ch_cdxAPI.downloadFile("https://web.archive.org/cdx/search?url=" + domain + "&fl=mimetype,timestamp,original", buff);
    if(downloadFileRes.errcode != curl_helper::errEnum::OK){
        return error{.errcode=CURL_FAIL, .errmsg=downloadFileRes.errmsg};
    }

    /*
        Parse the data into an std::vector of dbEntry structs.

        Expected format: mimetype timestamp url \n mimetype timestamp url \n ...
    */
    std::string cdxData;
    cdxData.assign(buff.begin(), buff.end()); // Convert to string
    std::vector<URLDBEntry> cdxDataParsed;
    std::istringstream cdp_iss(cdxData);
    std::string line;
    while(std::getline(cdp_iss, line)){
        std::istringstream l_iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while(std::getline(l_iss, token, ' ')){
            tokens.push_back(token);
        }
        if(tokens.size() != 3){
            return error{.errcode=API_BADDATA, .errmsg="Unexpected number of tokens in line while parsing cdxData"};
        }
        cdxDataParsed.push_back(URLDBEntry{
            .rowID = -1, // We dont insert rowID into the db manually, it being a primary key means sqlite does that work for us
            .url = tokens[2],
            .timestamp = tokens[1],
            .mimetype = tokens[0],
            .scraped = 0
        });
    }

    std::cout << cdxDataParsed.size() << std::endl;

    /*
        Insert the data into the DB
    */
    std::string insert_query = "INSERT INTO " + this->URLDB_tablename + " (url, wayback_timestamp, mimetype, scraped) VALUES ";
    for(int i=0; i<cdxDataParsed.size(); i++){
        insert_query += "('" + this->sanitiseStr(cdxDataParsed[i].url) + "', '" 
                             + this->sanitiseStr(cdxDataParsed[i].timestamp) + "', '" 
                             + this->sanitiseStr(cdxDataParsed[i].mimetype) + "', " 
                             + std::to_string(cdxDataParsed[i].scraped) + ((i==cdxDataParsed.size()-1)? ")" : "),");
    }

    return this->basicQueryExec(insert_query);
}


manticore::error manticore::manticoreDB::URLDB_enableTOR(const int port){
    curl_helper::error res = this->ch_cdxAPI.enableTOR(port);
    if(res.errcode == curl_helper::OK){
        return error{.errcode=OK, .errmsg=""};
    }
    return error{.errcode=GENERIC_ERR, .errmsg=res.errmsg};
}