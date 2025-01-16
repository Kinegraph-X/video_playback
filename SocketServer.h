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
    SocketServer(int port);
    ~SocketServer();

    bool start();
    void stop();
    std::string receiveCommand();

private:
    void listenForConnections();
    void handleClient(int clientSocket);

    int serverSocket;
    int port;
    std::atomic<bool> isRunning;

    std::thread listeningThread;
    std::vector<std::thread> clientThreads;

    std::mutex commandQueueMutex;
    std::condition_variable commandQueueCondition;
    std::queue<std::string> commandQueue;
};
