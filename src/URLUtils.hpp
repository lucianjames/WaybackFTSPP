#ifndef URLUTILS_HPP
#define URLUTILS_HPP

#include <string>


/*
    !!

    Copied from old project, not all of these are needed!!! Will clean up later

    !!
*/


namespace URLUtils{

    bool isWebURI(const std::string& url);
    bool isRelativeURL(const std::string& url);
    std::string urlRemoveFragment(std::string& url);
    std::string urlProcessPathSpecifiers(std::string url);
    std::string urlJoin(std::string base, std::string relative);
    std::string urlGetExtension(const std::string& url);
    std::string urlEncode(const std::string& url);
}

#endif