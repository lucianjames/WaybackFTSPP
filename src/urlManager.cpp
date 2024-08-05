#include "urlManager.hpp"

/*
    sqlite3 cleanup
*/
url_manager::urlDB::~urlDB(){
    if(db != NULL){
        sqlite3_close(this->db);
        //sqlite3_free(this->db);
    }
}


/*
    Creates a waybackFTSPP url db file and creates the URLs table
*/
url_manager::error url_manager::urlDB::create(const std::string& dbPath){
    if(std::filesystem::exists(dbPath)){
        return error{.errcode=FILE_EXISTS, .errmsg="file at dbPath already exists"};
    }

    if(sqlite3_open(dbPath.c_str(), &this->db)){
        return error{.errcode=SQLITE_ERR, .errmsg="Failed to create db"};
    }

    const std::string sql_create_table = "CREATE TABLE URLS(" \
                                            "ID INTEGER PRIMARY KEY, " \
                                            "URL TEXT NOT NULL, " \
                                            "TIMESTAMP TEXT NOT NULL, " \
                                            "MIMETYPE TEXT NOT NULL, " \
                                            "SCRAPED INT NOT NULL);"; // Used by scraper program to mark
    char* zErrMsg = 0; // Does this need freeing manually?
    if(sqlite3_exec(this->db, sql_create_table.c_str(), NULL, 0, &zErrMsg) != SQLITE_OK){
        std::string err = std::string(zErrMsg);
        free(zErrMsg);
        return error{.errcode=SQLITE_ERR, .errmsg=std::string("Failed to create table. SQL Error: ") + err};
    }
    free(zErrMsg); // Probably

    return error{.errcode=OK, .errmsg=""};
}


/*
    Opens an existing waybackFTSPP url db file
*/
url_manager::error url_manager::urlDB::open(const std::string& dbPath){
    if(!std::filesystem::exists(dbPath)){
        return error{.errcode=BAD_ARG, .errmsg="file at dbPath does not exist"};
    }
    if(sqlite3_open(dbPath.c_str(), &this->db)){
        return error{.errcode=SQLITE_ERR, .errmsg="Failed to open db"};
    }

    return error{.errcode=OK, .errmsg=""};
}


/*
    Gets available pages for a given domain from the wayback CDX API and adds it to the DB file
*/
url_manager::error url_manager::urlDB::addDomain(const std::string& domain){
    if(this->db == NULL){
        return error{.errcode=NOT_INIT, .errmsg="No DB file open"};
    }

    /*
        Download the list of pages from the API
    */
    std::vector<char> buff;
    std::cout << "Fetching page information from https://web.archive.org/cdx/search?url=" + domain + "&fl=mimetype,timestamp,original\n";
    curl_helper::error downloadFileRes = this->ch.downloadFile("https://web.archive.org/cdx/search?url=*." + domain + "/*&fl=mimetype,timestamp,original", buff);
    if(downloadFileRes.errcode != curl_helper::errEnum::OK){
        return error{.errcode=CURL_FAIL, .errmsg=downloadFileRes.errmsg};
    }
    std::cout << "Fetched page information. Parsing...\n";

    /*
        Parse the data into an std::vector of dbEntry structs.
    */
    std::string cdxData;
    cdxData.assign(buff.begin(), buff.end()); // Convert to string
    std::vector<dbEntry> cdxDataParsed;
    std::istringstream cdp_iss(cdxData);
    std::string line;
    while(std::getline(cdp_iss, line)){
        std::istringstream l_iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while(std::getline(l_iss, token, ' ')){
            tokens.push_back(token);
        }
        if(tokens.size() < 3){
            return error{.errcode=API_BADDATA, .errmsg="Unexpected number of tokens in line while parsing cdxData"};
        }
        cdxDataParsed.push_back(dbEntry{
            .rowID = -1, // We dont insert rowID into the db manually, it being a primary key means sqlite does that work for us
            .url = tokens[2],
            .timestamp = tokens[1],
            .mimetype = tokens[0],
            .scraped = 0
        });
    }

    /*
        Insert all the dbEntry structs into the sqlite db file
    */
    SQLITE_CALL(sqlite3_exec(this->db, "PRAGMA synchronous = OFF", NULL, NULL, NULL), this->db); // Removes safety and checks, makes things go zoom zoom
    SQLITE_CALL(sqlite3_exec(this->db, "BEGIN TRANSACTION", NULL, NULL, NULL), this->db);
    const char* sql_insert = "INSERT INTO URLS (URL, TIMESTAMP, MIMETYPE, SCRAPED) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    SQLITE_CALL(sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL), this->db);
    for(const auto& page : cdxDataParsed){
        sqlite3_bind_text(stmt, 1, page.url.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, page.timestamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, page.mimetype.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, 0);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    SQLITE_CALL(sqlite3_finalize(stmt), this->db);
    SQLITE_CALL(sqlite3_exec(this->db, "END TRANSACTION", NULL, NULL, NULL), this->db);
    
    return error{.errcode=OK, .errmsg=""};
}


/*
    Enable tor in this class's curl helper instance
*/
url_manager::error url_manager::urlDB::enableTOR(const int port){
    curl_helper::error res = this->ch.enableTOR(port);
    if(res.errcode == curl_helper::OK){
        return error{.errcode=OK, .errmsg=""};
    }
    return error{.errcode=GENERIC_ERR, .errmsg=res.errmsg};
}


/*
    If allowed_mimetypes is empty, no filtering will be supplied
*/
url_manager::error url_manager::urlDB::getData(std::vector<dbEntry>& out, bool unscraped_only, const std::vector<std::string>& allowed_mimetypes){
    if(this->db == NULL){
        return error{.errcode=NOT_INIT, .errmsg="No DB file open"};
    }

    const std::string get_all_sql = "SELECT * FROM 'URLS';";
    sqlite3_stmt* stmt;
    SQLITE_CALL(sqlite3_prepare_v2(this->db, get_all_sql.c_str(), -1, &stmt, NULL), this->db);

    while(sqlite3_step(stmt) == SQLITE_ROW){
        dbEntry e;
        e.rowID = sqlite3_column_int(stmt, 0);
        e.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        e.timestamp = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        e.mimetype = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        e.scraped = sqlite3_column_int(stmt, 4);

        bool allowed_mimetype = false;
        if(allowed_mimetypes.size() > 0){ // if allowed_mimetypes is empty, then no filtering is to be applied
            if(std::find(allowed_mimetypes.begin(), allowed_mimetypes.end(), e.mimetype) != allowed_mimetypes.end()){
                allowed_mimetype = true;
            }
        }
        if(allowed_mimetype && !((e.scraped == 1) && unscraped_only)){
            out.push_back(e);
        }

    }
    SQLITE_CALL(sqlite3_finalize(stmt), this->db);

    return error{.errcode=OK, .errmsg=""};
}


url_manager::error url_manager::urlDB::setScraped(const int ID, const bool val){

    const std::string sql_update_record = std::string("UPDATE URLS " \
                                                      "SET SCRAPED = ") + (val? "1" : "0") + " WHERE ID = " + std::to_string(ID) + ";";

    char* zErrMsg = 0;
    if(sqlite3_exec(this->db, sql_update_record.c_str(), NULL, 0, &zErrMsg) != SQLITE_OK){
        std::string err = std::string(zErrMsg);
        free(zErrMsg);
        return error{.errcode=SQLITE_ERR, .errmsg=std::string("Failed to update record. SQL Error: ") + err};
    }
    free(zErrMsg);

    return error{.errcode=OK, .errmsg=""};
}