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
        return error{.errcode=BAD_ARG, .errmsg="file at dbPath already exists"};
    }

    if(sqlite3_open(dbPath.c_str(), &this->db)){
        return error{.errcode=SQLITE_ERR, .errmsg="Failed to create db"};
    }

    char* zErrMsg = 0; // Does this need freeing manually?
    if(sqlite3_exec(this->db, this->sql_create_table.c_str(), NULL, 0, &zErrMsg) != SQLITE_OK){
        return error{.errcode=SQLITE_ERR, .errmsg=std::string("Failed to create table. SQL Error: ") + std::string(zErrMsg)};
    }
    free(zErrMsg);

    return error{.errcode=OK, .errmsg=""};
}


#define SQLITE_CALL(func, db, ...) \
    do { \
        int rc = func(db, __VA_ARGS__); \
        if (rc != SQLITE_OK) { \
            fprintf(stderr, "SQLite error in %s: %s\n", #func, sqlite3_errmsg(db)); \
            sqlite3_close(db); \
            exit(1); \
        } \
    } while(0)



void handleSQLiteError(sqlite3* db, int rc) {
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
}


/*
    Gets available pages for a given domain from the wayback CDX API and adds it to the DB file
*/
url_manager::error url_manager::urlDB::addDomain(const std::string& domain){
    if(this->db == NULL){
        return error{.errcode=NOT_INIT, .errmsg="No DB file open"};
    }

    //https://web.archive.org/cdx/search?url=*.domain.org/*&fl=mimetype,timestamp,original
    this->ch.enableTOR(9055); // This should go elsewhere, im just testingggg
    std::vector<char> buff;
    std::cout << "https://web.archive.org/cdx/search?url=*." + domain + "/*&fl=mimetype,timestamp,original\n";
    this->ch.downloadFile("https://web.archive.org/cdx/search?url=*." + domain + "/*&fl=mimetype,timestamp,original", buff);

    // conv to str, the api should never return any bad chars so no extra handling needed
    std::string cdxData;
    cdxData.assign(buff.begin(), buff.end());

    // Parese into std::vector<dbEntry>
    // TODO ADD ERR HADNLING BIOGORHGUIYH
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
        dbEntry e = dbEntry{
            .ID = -1,
            .url = tokens[2],
            .timestamp = tokens[1],
            .mimetype = tokens[0],
            .scraped = 0
        };
        cdxDataParsed.push_back(e);
        std::cout << e.url << std::endl;
    }

    // Insert entries into DB
    int res = sqlite3_exec(this->db, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
    handleSQLiteError(this->db, res);
    res = sqlite3_exec(this->db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    handleSQLiteError(this->db, res);
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO URLS (URL, TIMESTAMP, MIMETYPE, SCRAPED) VALUES (?, ?, ?, ?);";
    res = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    handleSQLiteError(this->db, res);
    for(const auto& page : cdxDataParsed){
        sqlite3_bind_text(stmt, 1, page.url.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, page.timestamp.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, page.mimetype.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, 0);
        res = sqlite3_step(stmt);
        //handleSQLiteError(this->db, res);
        res = sqlite3_reset(stmt);
        //handleSQLiteError(this->db, res);
    }
    res = sqlite3_finalize(stmt);
    handleSQLiteError(this->db, res);
    res = sqlite3_exec(this->db, "END TRANSACTION", NULL, NULL, NULL);
    handleSQLiteError(this->db, res);

    return error{.errcode=OK, .errmsg=""};
}