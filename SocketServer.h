#pragma once

#include "player_headers.h"
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <winsock2.h>
#include <ws2tcpip.h>

class SocketServer {
public:
    SocketServer(unsigned short port, std::atomic<bool>& runningFlag);
    ~SocketServer();

    bool start();
    std::string receiveCommand();
    void stop();
    void reset();

private:
    void listenForConnections();
    void handleClient(SOCKET clientSocket);

    SOCKET serverSocket;
    int port;
    std::atomic<bool>& isRunning;
    bool isAborted = false;

    std::thread listeningThread;
    std::vector<std::thread> clientThreads;

    std::mutex commandQueueMutex;
    std::condition_variable commandQueueCondition;
    std::queue<std::string> commandQueue;

    void cleanup();
};
