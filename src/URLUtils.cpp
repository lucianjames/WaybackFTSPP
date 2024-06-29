#include "URLUtils.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>


/*
    !!

    Copied from old project, not all of these are needed!!! Will clean up later

    !!
*/


bool URLUtils::isWebURI(const std::string& uri) {
    bool hasHttp = uri.find("http://") != std::string::npos || uri.find("https://") != std::string::npos;
    bool containsMailto = uri.find("mailto:") != std::string::npos;
    bool containsTel = uri.find("tel:") != std::string::npos;
    bool containsJavascript = uri.find("javascript:") != std::string::npos;
    return hasHttp && !containsMailto && !containsTel && !containsJavascript;
}


bool URLUtils::isRelativeURL(const std::string& url){
    return url.find("://") == std::string::npos;
}


std::string URLUtils::urlRemoveFragment(std::string& url){
    size_t pos = url.find('#');
    if(pos == std::string::npos){
        return url;
    }
    return url.substr(0, pos);
}


std::string URLUtils::urlProcessPathSpecifiers(std::string url){ // Not passed by ref because might not want to destroy the original value
    std::vector<std::string> components;
    size_t pos = url.find("://");
    if(pos == std::string::npos){
        pos = 0;
    }else{
        pos += 3; // protocolPos = http(s) (or whatever protocol) plus the 3 chars for :// after this
    }
    
    pos = url.find('/', pos);
    for(;;){
        if(pos == std::string::npos){
            if(url.size() > 0){
                components.push_back(url);
            }
            break;
        }
        components.push_back(url.substr(0, pos));
        url = url.substr(pos+1);
        pos = url.find('/');
    }
    std::string base = components[0];
    components.erase(components.begin()); // Remove the base

    // Process the components
    // .. -> remove the previous component (and the ..)
    // . -> remove the .
    for(size_t i=0; i<components.size(); i++){
        if(components[i] == ".."){
            if(i > 0){
                components.erase(components.begin()+i-1, components.begin()+i+1); // Remove the '..' and the directory before it
                i-=2;
            }else{
                components.erase(components.begin()+i); // Remove the '..'
                i-=1;
            }
        }
        if(components[i] == "."){
            components.erase(components.begin()+i); // Remove the '.'
            i-=1;
        }
    }

    // Rebuild the url from the processed components
    std::string processedURL = base;
    for(auto& c : components){
        processedURL += "/" + c;
    }
    return processedURL;
}


std::string URLUtils::urlJoin(std::string base, std::string relative) { // Not passed by reference because might not want to modify the original values
    if (relative.empty() || relative[0] == '#') {
        return base;
    }

    if(relative[0] == '/'){
        size_t pos = base.find("//"); // Skip past the protocol in the base URL
        if (pos != std::string::npos) {
            pos = base.find('/', pos + 2); // Get the position of the first '/' after the domain
            if (pos != std::string::npos) {
                base = base.substr(0, pos); // Remove everything past the domain
            }
        }
        return base + relative;
    }

    base = base.substr(0, base.rfind('/')+1); // Remove the file name from the base URL
    return base + relative;
}


// Given a URL, return the extension of the file it points to
std::string URLUtils::urlGetExtension(const std::string& url){
    size_t pos = url.rfind('.');
    if(pos == std::string::npos){
        return "";
    }
    std::string ext = url.substr(pos+1);
    for(auto& c : ext){
        c = tolower(c);
    }
    return ext;
}


std::string URLUtils::urlEncode(const std::string& url){
    std::ostringstream encoded;
    encoded.fill('0'); // Makes sure theres no padding on hex stuff
    encoded << std::hex;

    for (char c : url) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else if (c == ' ') {
            encoded << '+';
        } else {
            encoded << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}