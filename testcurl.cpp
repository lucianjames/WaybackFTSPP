#include "src/curlHelper.hpp"

int main(){

    curl_helper::curlHelper ch;
    std::vector<char> rb;
    ch.downloadFile("https://www.example.com", rb);

    return 0;
}