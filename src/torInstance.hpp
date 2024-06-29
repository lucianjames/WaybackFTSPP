#ifndef TORINSTANCE_HPP
#define TORINSTANCE_HPP

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

    enum errEnum : char {
        OK,
        GENERIC_ERR,
    };

    struct error {
        errEnum errcode;
        std::string errmsg;
    };

    class torInstance{
    private:
        int port = 9051;
        std::string torrcPath = "./torrc9051";
        pid_t torProxyPID = -1;
        void createTorrc();
        void waitForTor();

    public:
        torInstance();
        torInstance(int port);
        ~torInstance();
        void setPort(int port);
        int getPort();
        error start();
        void stop();
        error restart();
        bool isRunning();
    };

}

#endif