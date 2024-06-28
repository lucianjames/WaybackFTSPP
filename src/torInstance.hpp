#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <signal.h>



namespace TOR{

    class torInstance{
    private:
        int port = 9050;
        std::string torrcPath = "./torrc9050";
        pid_t torProxyPID = -1;
        void createTorrc();
        void waitForTor();

    public:
        torInstance();
        torInstance(int port);
        ~torInstance();
        void setPort(int port);
        int getPort();
        void start();
        void stop();
        void restart();
        bool isRunning();
    };

}