#include "curlHelper.hpp"


size_t curl_helper::curlHelper::curlWriteCallback_bin(void *contents, size_t size, size_t nmemb, void *userp) {
    std::vector<char>* vec = (std::vector<char>*)userp;
    vec->insert(vec->end(), (char*)contents, (char*)contents+(size*nmemb));
    return size*nmemb;
}

size_t curl_helper::curlHelper::curlWriteCallback_str(char* contents, size_t size, size_t nmemb, void* userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


curl_helper::error curl_helper::curlHelper::enableTOR(const int port){
    this->torInstance.setPort(port);
    TOR::error res = this->torInstance.start();
    if(res.errcode == TOR::OK){
        return error{.errcode=OK, .errmsg=""};
    }else{
        return error{.errcode=TOR_FAIL, .errmsg=res.errmsg};
    }
}


curl_helper::error curl_helper::curlHelper::downloadFile(const std::string& url, std::vector<char>& curlReadBuffer, int retries){
    int retriesStart = retries;
    bool success = false;
    while(!success){
        CURL* curlPtr = curl_easy_init();
        if(curlPtr == NULL){
            return error{.errcode=CURL_ERR, .errmsg="Failed to init CURL"};
        }
        curl_easy_setopt(curlPtr, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curlPtr, CURLOPT_USERAGENT, this->userAgent.c_str());
        curl_easy_setopt(curlPtr, CURLOPT_WRITEFUNCTION, curl_helper::curlHelper::curlWriteCallback_bin);
        curl_easy_setopt(curlPtr, CURLOPT_WRITEDATA, &curlReadBuffer);
        curl_easy_setopt(curlPtr, CURLOPT_FOLLOWLOCATION, 1L);
        if(this->useTor){
            std::string tor = "socks5h://127.0.0.1:" + std::to_string(this->torInstance.getPort());
            curl_easy_setopt(curlPtr, CURLOPT_PROXY, tor.c_str());
        }

        CURLcode res = curl_easy_perform(curlPtr);
        success = (res == CURLE_OK);
        curl_easy_cleanup(curlPtr);
        if(!success){
            if(retries > 0){
                int timeout = this->failTimeout * (retriesStart - retries + 1);
                std::cout << "Download failed for " << url << ", retrying in " << timeout << " seconds. (" << retries << " retries left)" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
                retries--;
            }else{
                return error{.errcode=TIMEOUT, .errmsg="Couldnt download the given file within the given number of retries:" + std::string(curl_easy_strerror(res))};
            }
        }
    }
    return error{.errcode=OK, .errmsg=""};
}

curl_helper::error curl_helper::curlHelper::manticoreQuery(const std::string& serverAddr, const std::string& query, std::string& readBuffer){
    CURL* curlPtr = curl_easy_init();
    if(curlPtr == NULL){
        return error{.errcode=CURL_ERR, .errmsg="Failed to init CURL"};
    }
    std::string URL = serverAddr + "/sql?mode=raw";
    std::string payload = "query=" + URLUtils::urlEncode(query);
    curl_easy_setopt(curlPtr, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curlPtr, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curlPtr, CURLOPT_POSTFIELDSIZE, payload.size());
    curl_easy_setopt(curlPtr, CURLOPT_POST, 1L);
    curl_easy_setopt(curlPtr, CURLOPT_WRITEFUNCTION, curl_helper::curlHelper::curlWriteCallback_str);
    curl_easy_setopt(curlPtr, CURLOPT_WRITEDATA, &readBuffer);
    CURLcode res = curl_easy_perform(curlPtr);
    curl_easy_cleanup(curlPtr);
    if(res != CURLE_OK){
        return error{.errcode=CURL_ERR, .errmsg="Curl error: " + std::string(curl_easy_strerror(res))};
    }
    return error{.errcode=OK, .errmsg=""};
}