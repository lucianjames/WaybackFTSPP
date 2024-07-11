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

curl_helper::error curl_helper::curlHelper::htmlTreeGetTitle(xmlDoc* doc, xmlNodePtr node, std::string& titleBuff){
    xmlNodePtr currentNode = NULL;
    for(currentNode = node; currentNode; currentNode = currentNode->next){ // For every node in the tree
        if(currentNode->type == XML_ELEMENT_NODE && !xmlStrcmp(currentNode->name, (const xmlChar*)"title")){
            titleBuff = (char*)xmlNodeListGetString(doc, currentNode->xmlChildrenNode, 1);
            return error{.errcode=OK, .errmsg=""};
        }
        if(currentNode->children != NULL){
            this->htmlTreeGetTitle(doc, currentNode->children, titleBuff); // Recurse
        }
    }
    return error{.errcode=OK, .errmsg="No title found"};
}

curl_helper::error curl_helper::curlHelper::htmlTreeGetText(xmlNodePtr node, std::string& textBuff){
    xmlNodePtr curNode = NULL;
    for(curNode = node; curNode; curNode = curNode->next){ // For each node in the tree
        if(curNode->type == XML_TEXT_NODE){ // If this node is a text node, slap its content into the buffer
            textBuff += (char*)curNode->content;
        }
        if(curNode->children != NULL){
            this->htmlTreeGetText(curNode->children, textBuff); // Recurse
        }
    }
    return error{.errcode=OK, .errmsg=""};
}

/*
    !!! Remember to free parser with xmlFreeDoc(parser->myDoc); htmlFreeParserCtxt(parser);
*/
curl_helper::error curl_helper::curlHelper::htmlParse(const std::string& pageSrc, htmlParserCtxtPtr& parser){
    parser = htmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL, XML_CHAR_ENCODING_NONE);
    if(parser == NULL){
        return error{.errcode=LIBXML_FAIL, .errmsg="Failed to create libxml2 HTML parser"};
    }
    if(htmlCtxtUseOptions(parser, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING) != 0){
        return error{.errcode=LIBXML_FAIL, .errmsg="Failed to set parser options"};
    }
    htmlParseChunk(parser, pageSrc.c_str(), pageSrc.size(), 0);
    htmlParseChunk(parser, NULL, 0, 1);
    if(parser->myDoc == NULL){
        xmlFreeDoc(parser->myDoc);
        htmlFreeParserCtxt(parser);
        return error{.errcode=LIBXML_NULLDOC, .errmsg="Null document"};
    }
    return error{.errcode=OK, .errmsg=""};
}

curl_helper::error curl_helper::curlHelper::getTitleFromPage(const std::string& pageSrc, std::string& titleOut){
    htmlParserCtxtPtr parser;
    error parseRes = this->htmlParse(pageSrc, parser);
    if(parseRes.errcode != OK){
        return parseRes;
    }

    titleOut = "";
    this->htmlTreeGetTitle(parser->myDoc, parser->myDoc->children, titleOut);

    xmlFreeDoc(parser->myDoc);
    htmlFreeParserCtxt(parser);
    return error{.errcode=OK, .errmsg=""};
}

curl_helper::error curl_helper::curlHelper::getTextFromPage(const std::string& pageSrc, std::string& textOut){
    htmlParserCtxtPtr parser;
    error parseRes = this->htmlParse(pageSrc, parser);
    if(parseRes.errcode != OK){
        return parseRes;
    }

    textOut = "";
    this->htmlTreeGetText(parser->myDoc->children, textOut);

    xmlFreeDoc(parser->myDoc);
    htmlFreeParserCtxt(parser);
    return error{.errcode=OK, .errmsg=""};
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

curl_helper::error curl_helper::curlHelper::getParsedPage(const std::string& url, curl_helper::parsedPage& out){
    std::vector<char> buff;
    this->downloadFile(url, buff);
    
    std::string page;
    page.assign(buff.begin(), buff.end()); // Convert to std::string, assumes valid plaintext html
    out.raw = page;

    this->getTitleFromPage(page, out.title);
    this->getTextFromPage(page, out.text);

    return error{.errcode=OK, .errmsg=""};
}