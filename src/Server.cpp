#include "Server.hpp"

bool Server::_signal = false;

Server::Server(int port, const std::string& password) : _port(port), _serverSocketFd(-1), _password(password) {}

Server::~Server() {}

std::string Server::getPassword() const {
    return _password;
}

void Server::receiveSignal(int signum) {
    _signal = true;
    (void)signum;
}

Client& Server::getClientByFd(int fd) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i].getFd() == fd)
            return _clients[i];
    }
    // This should never happen if the fd is valid
    // But we need to return something to avoid undefined behavior
    throw std::runtime_error("Client not found");
}

void Server::setNickname(int fd, const std::string& nickname) {nicknames[fd] = nickname;}

void Server::setUsernames(int fd, const std::string& username) {usernames[fd] = username;}

void Server::run() {
    signal(SIGINT, receiveSignal);
    signal(SIGQUIT, receiveSignal);

    // Create socket
    this->_serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocketFd == -1)
        throw std::runtime_error("Error: failed to create socket");
    
    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(_serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Error: setsockopt() failed");
    
    // Make non-blocking
    if (fcntl(_serverSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Error: fcntl() failed");
    
    // Bind
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);
    if (bind(_serverSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        throw std::runtime_error("Error: failed to bind socket");
    
    // Listen
    if (listen(_serverSocketFd, SOMAXCONN) == -1)
        throw std::runtime_error("Error: listen() failed");
    
    // Add to poll
    addPollfd(_serverSocketFd);
    
    std::cout << GREEN << ">>> SERVER STARTED <<<" << RESET << std::endl;
    std::cout << CYAN <<"Waiting for connections..." << RESET << std::endl;
    
    // Main event loop
    while (!_signal) {
        int ret = poll(&_fds[0], _fds.size(), -1);
        if (ret == -1 && !_signal)
            throw std::runtime_error("Error: poll() failed");

        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocketFd)
                    handleClientConnection();
                else
                    handleClientData(_fds[i].fd);
            }
        }
    }
    closeFds();
}

void Server::addPollfd(int fd) {
    struct pollfd newPollfd;
    newPollfd.fd = fd;
    newPollfd.events = POLLIN;
    newPollfd.revents = 0;
    _fds.push_back(newPollfd);
}

int Server::sendData(int fd, const std::string& message)
{
    size_t total_sent = 0;
    size_t message_length = message.length();
    
    while (total_sent < message_length)
    {
        ssize_t bytes_sent = send(fd, message.c_str() + total_sent, 
                                   message_length - total_sent, 0);
        if (bytes_sent < 0)
        {
            std::cerr << "Error: send failed for client FD " << fd << std::endl;
            return -1;
        }
        else if (bytes_sent == 0)
        {
            std::cerr << "Connection closed by peer for client FD " << fd << std::endl;
            return -1;
        }
        total_sent += bytes_sent;
    }
    return total_sent;
}

void Server::handleClientConnection() {
    struct sockaddr_in client_addr;
    socklen_t clientAddrSize = sizeof(sockaddr_in);
    int newFd = accept(_serverSocketFd, (struct sockaddr *)&client_addr, &clientAddrSize);
    if (newFd == -1)
        throw std::runtime_error("Error: accept() failed");

    if (fcntl(newFd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("Error: fcntl() failed");

    std::string passwordRequest = "Please Enter The password Of This Server :\n";
    std::string art = MINICHAT;

    sendData(newFd, art);
    sendData(newFd, passwordRequest);
    addPollfd(newFd);
    _clients.push_back(Client(newFd));
        
    std::cout << "Client <" << newFd << "> Connected" << std::endl;
}

void Server::sendJoinMsg(const std::string& nickname, const std::string& channelName, int fd) {
    // Get operators from the channel's operator map instead of global variables
    std::string operators;
    std::string operators1;
    
    const std::map<std::string, int>& ops = channels[channelName].getOperators();
    if (!ops.empty()) {
        operators = ops.begin()->first;  // First operator
        if (ops.size() > 1) {
            std::map<std::string, int>::const_iterator it = ops.begin();
            ++it;
            operators1 = it->first;  // Second operator if exists
        }
    }

    std::string topic = channels[channelName].getTopic();
    std::string creationTimeMessage = constructJoinedTimeMessage(channelName);
    std::string joinMessage = JOIN_MESSAGE(nickname, channelName);
    std::string topicMessage = TOPIC_MESSAGE(nickname, channelName, topic);

    sendData(fd, joinMessage);
    sendData(fd, topicMessage);

    std::string namesMessage = NAMES_MESSAGE2(nickname, channelName);
    const std::vector<std::string>& clients = channels[channelName].getClients();
    for (size_t i = 0; i < clients.size(); ++i) {
        const std::string& user = clients[i];
        if (user == operators || user == operators1) {
            namesMessage += "@" + user;
        } else {
            namesMessage += user;
        }
        if (i < clients.size() - 1)
            namesMessage += " ";
    }
    namesMessage += "\r\n";
    std::string endOfNamesMessage = END_OF_NAMES_MESSAGE(nickname, channelName);
    std::string channelMessage = CHANNEL_MESSAGE(channelName, creationTimeMessage);

    sendData(fd, namesMessage);
    sendData(fd, endOfNamesMessage);
    sendData(fd, channelMessage);

    smallbroadcastMessageforjoin(nickname, channelName);
}

// handling private msg between users only 
void Server::handlePrivateMessage(int senderFd, const std::string& recipient, const std::string& message) {
    int recipientFd = findUserFd1(recipient);
    if (recipientFd != -1) {
        std::string privateMessage = PRIVATE_MESSAGE(senderFd, recipient, message);
        sendData(recipientFd, privateMessage);
    } else {
        std::string errorMessage = ERROR_MESSAGE(senderFd, recipient);
        sendData(senderFd, errorMessage);
    }
}

void Server::handleInvitation(int senderFd, const std::string& recipient, std::string channelName) {
    int recipientFd = findUserFd1(recipient);

    if (recipientFd != -1) {
        std::string inviteMessage = INVITE_MESSAGE(senderFd, recipient, channelName);
        sendData(recipientFd, inviteMessage);
    } else {
        std::string errorMessage = ERROR_MESSAGE(senderFd, recipient);
        sendData(senderFd, errorMessage);
    }
}

void Server::handleClientData(int fd)
{
    Client& client = getClientByFd(fd); 
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = recv(fd, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        client.appendCommand(buffer);
        
        // Process ALL complete commands in buffer
        std::string cmdBuffer = client.getCommand();
        size_t newlinePos;
        while ((newlinePos = cmdBuffer.find_first_of("\r\n")) != std::string::npos) {
            std::string command = cmdBuffer.substr(0, newlinePos);
            
            // Remove the processed command and handle CRLF properly
            size_t nextPos = newlinePos + 1;
            
            // Skip over \r\n or \n\r sequences
            if (nextPos < cmdBuffer.length()) {
                if ((cmdBuffer[newlinePos] == '\r' && cmdBuffer[nextPos] == '\n') ||
                    (cmdBuffer[newlinePos] == '\n' && cmdBuffer[nextPos] == '\r')) {
                    nextPos++;
                }
            }
            
            cmdBuffer = (nextPos < cmdBuffer.length()) ? cmdBuffer.substr(nextPos) : "";
            client.setCommand(cmdBuffer);
            
            std::cout << "Received data from client " << fd << ": " << command << std::endl;
            
            // Process command (existing logic)
            int auth = client.getAuthentication();
            if ((startsWith(command, "PASS ") || startsWith(command, "pass ")) && auth == 0)
                processPassword(client, command, fd);
            else if ((startsWith(command, "NICK ") || startsWith(command, "nick ")) && auth == 1)
                processNickCmd(client, command, fd);
            else if ((startsWith(command, "USER ") || startsWith(command, "user ")) && auth == 2)
                processUserCmd(client, command, fd);
            else if (auth == 3) {
                if (startsWith(command, "JOIN ") || startsWith(command, "join "))
                    processJoinCmd(client, command, fd);
                else if (startsWith(command, "PRIVMSG ") || startsWith(command, "privmsg "))
                    processPrivmsgCmd(client, command, fd);
                else if (startsWith(command, "KICK ") || startsWith(command, "kick "))
                    processKickCmd(client, command, fd);
                else if (startsWith(command, "TOPIC ") || startsWith(command, "topic "))
                    processTopicCmd(client, command, fd);
                else if (startsWith(command, "INVITE ") || startsWith(command, "invite "))
                    processInviteCmd(client, command, fd);
                else if (startsWith(command, "BOT ") || startsWith(command, "bot "))
                    processBotCmd(client, command, fd);
                else if (startsWith(command, "MODE ") || startsWith(command, "mode "))
                    processModeCmd(client, command, fd);
                else if (startsWith(command, "QUIT") || startsWith(command, "quit"))
                    processQuitCmd(fd);
                else if (startsWith(command, "PING"))
                    ping(command, fd);
            }
        }
    }
    else if (bytesRead == 0) {
        std::cout << "Client <" << fd << "> Disconnected" << std::endl;
        cleanChannel(fd);
        clientCleanup(fd);
    } else if (bytesRead == -1) {
        std::cerr << "Error reading data from client <" << fd << ">" << std::endl;
        cleanChannel(fd);
        clientCleanup(fd);
    }
}
