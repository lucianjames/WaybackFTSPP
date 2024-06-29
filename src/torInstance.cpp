#include "torInstance.hpp"

#include <filesystem>


void TOR::torInstance::createTorrc(){
    if(!std::filesystem::exists("./torData")){
        std::filesystem::create_directory("./torData");
    }
    std::ofstream torrcFile(this->torrcPath);
    torrcFile << "SocksPort " << this->port << std::endl;
    torrcFile << "DataDirectory ./torData/torData" + std::to_string(this->port) << std::endl;
    //torrcFile << "Log notice file /dev/null" << std::endl; // Don't display those ugly warn/info messages // Keeping them on for now for debugging
    torrcFile.close();
}


void TOR::torInstance::waitForTor(){
    // Try to connect to 127.0.0.1:this->port using a socket every 100ms until it works:
    int sFD = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    while(connect(sFD, (struct sockaddr*)&addr, sizeof(addr)) != 0){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

TOR::torInstance::torInstance(){
    this->setPort(9051);
}


TOR::torInstance::torInstance(int port){
    this->setPort(port);
}


TOR::torInstance::~torInstance(){
    this->stop();
}


void TOR::torInstance::setPort(int port){
    this->port = port;
    this->torrcPath = "./torData/torrc" + std::to_string(port);
}


int TOR::torInstance::getPort(){
    return this->port;
}


TOR::error TOR::torInstance::start(){
    this->createTorrc(); // Ensure that the torrc file with the correct port configured exists
    FILE* torInstallCheck = popen("tor --version", "r"); // Check if TOR can be run from the command line
    if(torInstallCheck == NULL){
        return error{.errcode=GENERIC_ERR, .errmsg="TOR is not installed on this system"};
    }
    pclose(torInstallCheck);
    this->torProxyPID = fork();
    if(this->torProxyPID == 0){ // Inside the child process:
        execlp("tor", "tor", "-f", this->torrcPath.c_str(), NULL);
        throw std::runtime_error("Failed to star TOR"); // This makes sense I think
    }
    // Inside the parent process:
    std::cout << "Starting TOR on port " << this->port << "\n";
    this->waitForTor(); // Wait for TOR to start up
    std::cout << "TOR started on port " << this->port << " (successful socket connection made)\n";
    return error{.errcode=OK, .errmsg=""};
}


void TOR::torInstance::stop(){
    if(this->torProxyPID != -1){
        kill(this->torProxyPID, SIGTERM);
        this->torProxyPID = -1;
    }
}


TOR::error TOR::torInstance::restart(){
    this->stop();
    return this->start();
}


bool TOR::torInstance::isRunning(){
    return (this->torProxyPID != -1);
}