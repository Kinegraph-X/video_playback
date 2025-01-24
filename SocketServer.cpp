#include "SocketServer.h"




SocketServer::SocketServer(unsigned short port) : port(port), serverSocket(-1), isRunning(false) {}

SocketServer::~SocketServer() {
    cleanup();
}

bool SocketServer::start() {
//    if (isRunning) return false;

    // Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logger(LogLevel::ERR, "WSAStartup failed");
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        logger(LogLevel::ERR, "Socket creation failed");
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;	// inet_addr("127.0.0.1");//
    logger(LogLevel::DEBUG, "Trying to bind socket on port : " + LogUtils::toString(port));
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logger(LogLevel::ERR, "Socket binding failed : " + LogUtils::toString(WSAGetLastError()));
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket, MAX_CONNECTIONS) == SOCKET_ERROR) {
        logger(LogLevel::ERR, "Socket listen failed : " + LogUtils::toString(WSAGetLastError()));
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    isRunning = true;
    listeningThread = std::thread(&SocketServer::listenForConnections, this);
    return true;
}


void SocketServer::listenForConnections() {
	int err;
    while (isRunning) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == -1) {
			err = WSAGetLastError();
	        if (isRunning) logger(LogLevel::ERR, "Socket accept failed" + LogUtils::toString(err));
	        continue;
	    }

        clientThreads.emplace_back(&SocketServer::handleClient, this, clientSocket);
    }
}

void SocketServer::handleClient(SOCKET clientSocket) {
    char buffer[1024];

    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead < 0) {
            closesocket(clientSocket);
            break;
        }

        {
            std::lock_guard<std::mutex> lock(commandQueueMutex);
            commandQueue.emplace(buffer);
        }

        commandQueueCondition.notify_one();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::string SocketServer::receiveCommand() {
    std::unique_lock<std::mutex> lock(commandQueueMutex);
    commandQueueCondition.wait(lock, [this] { return !commandQueue.empty() || !isRunning; });

    if (!commandQueue.empty()) {
        std::string command = commandQueue.front();
        commandQueue.pop();
        return command;
    }

    return {};
}

void SocketServer::stop() {
	std::lock_guard<std::mutex> lock(commandQueueMutex);
	if (isRunning) {
		isRunning = false;
		commandQueueCondition.notify_all();
	}
}

void SocketServer::cleanup() {
    logger(LogLevel::DEBUG, "SocketServer::cleanup starting");
	
	if (!isAborted) {
	    // First notify all waiting threads
		stop();
	
	    // Give threads a moment to see the isRunning flag
	    std::this_thread::sleep_for(std::chrono::milliseconds(30));
	
	    // Then close the server socket
	    int ret = closesocket(serverSocket);
	    if (ret == -1) {
	        int err = WSAGetLastError();
	        logger(LogLevel::ERR, "SocketServer::cleanup Socket close failed: " + LogUtils::toString(err));
	    }
	
	    if (listeningThread.joinable()) {
	        logger(LogLevel::DEBUG, "SocketServer::cleanup Joining listening thread");
	        listeningThread.join();
	    }
	
	    for (auto& clientThread : clientThreads) {
	        if (clientThread.joinable()) {
	            clientThread.join();
	        }
	    }
	
	    clientThreads.clear();
	    
	    // Clear any remaining commands
	    {
	        std::lock_guard<std::mutex> lock(commandQueueMutex);
	        std::queue<std::string>().swap(commandQueue);  // Clear queue
	    }
		
		isAborted = true;
	}
    logger(LogLevel::DEBUG, "SocketServer::cleanup sequence ended");
}

void SocketServer::reset() {
    cleanup();  // Call cleanup to reset resources
}