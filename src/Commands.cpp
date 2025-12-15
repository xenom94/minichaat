#include "Server.hpp"

void Server::ping(const std::string& command, int fd) {
    std::istringstream iss(command);
    std::string serverHostname = command.substr(5);
    std::string pongMessage = "PONG " + serverHostname + "\r\n";
    sendData(fd, pongMessage);
    std::cout << "PONG" << std::endl;
}

void Server::processJoinCmd(Client& client, const std::string& command, int fd)
{
    std::string nick;
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i].getFd() == fd) {
            nick = _clients[i].getNick();
            break;
        }
    }
    std::string channelName, pass, what;
    std::istringstream iss(command.substr(5));
    iss >> channelName ;
    if (channelName[0] != '#')
    {
        what = " :Error: Channel start with #\r\n";
        std::string errorMessage = ERROR_MESSAGE2(nick);
            sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }
    channelName = channelName.substr(1);
    channelName = trim(channelName);
    std::getline(iss, pass);
    pass = trim(pass);

    // Check if the channel already exists
    std::map<std::string, Channel>::iterator it = channels.find(channelName);
    if (it != channels.end()) 
    {
        // Channel already exists
        if ((channels[channelName].isInviteOnly() && channels[channelName].isInvited(nick)) || channels[channelName].isOperator(fd))
        {
            // User is invited, create the channel
            int check = channels[channelName].getChannelLimit();
            if (channels[channelName].getCurrentUserCount() < check || !channels[channelName].hasUserLimit())
                createChannel(channelName, nick, fd);
            else {
                what = " :Error: CHANNEL limit\r\n";
                std::string errorMessage = ERROR_MESSAGE2(nick);
                sendData(fd, errorMessage);
            }
        } 
        else if (!channels[channelName].isInviteOnly()) {
            if (channels[channelName].hasPasswordSet() && channels[channelName].getPass() == pass) {
                int check = channels[channelName].getChannelLimit();
                if (channels[channelName].getCurrentUserCount() < check || !channels[channelName].hasUserLimit())
                    createChannel(channelName, nick, fd);
                else {
                    what = " :Error: CHANNEL limit\r\n";
                    std::string errorMessage = ERROR_MESSAGE2(nick);
                    sendData(fd, errorMessage);
                }
            }
            else if (!channels[channelName].hasPasswordSet()) {
                int check = channels[channelName].getChannelLimit();
                if (channels[channelName].getCurrentUserCount() < check || !channels[channelName].hasUserLimit())
                    createChannel(channelName, nick, fd);
                else {
                    what = " :Error: CHANNEL limit\r\n";
                    std::string errorMessage = ERROR_MESSAGE2(nick);
                    sendData(fd, errorMessage);
                }
            }
            else if (channels[channelName].hasPasswordSet() && channels[channelName].getPass() != pass) {
                what = " :Error: you need a password for this channel\r\n";
                std::string errorMessage = ERROR_MESSAGE2(nick);
                sendData(fd, errorMessage);
            }

        }
        else {
            // User is not invited, send error message
            what = " :Error: you are not invited\r\n";
            std::string errorMessage = ERROR_MESSAGE2(nick);
            sendData(fd, errorMessage);
        }
    } 
    else 
        createChannel(channelName, nick, fd);
}

void Server::processPrivmsgCmd(Client& client, const std::string& command, int fd)
{
    std::istringstream iss(command);
    std::string cmd, recipient, message;
    std::string niiick;

    iss >> cmd >> recipient;
    recipient = trim(recipient);
    std::getline(iss, message);
    
    if (iss.fail()) {
        std::string errorMessage = "Error: You Just missing an argument(5)\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }
    
    message = trim(message);
    message = message.substr(1);
    if (recipient[0] == '#') {
        recipient = recipient.substr(1);
        for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i].getFd() == fd) {
                niiick =  _clients[i].getNick();
                break;
            }
        }
        broadcastMessage(recipient, niiick, message);
    }
    else {
        handlePrivateMessage(fd, recipient, message);
    }
}

void Server::processQuitCmd(int fd) {
    cleanChannel(fd);
    clientCleanup(fd);
    std::cout << "Client <" << fd << "> Disconnected" << std::endl;
}

void Server::processKickCmd(Client& client, const std::string& command, int fd)
{
    std::string channelName, userToKick, reason;
    std::istringstream iss(command.substr(5));
    iss >> channelName >> userToKick;  

    if (iss.fail()) {
        std::string errorMessage = "Error: You Just missing an argument(4)\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    std::getline(iss, reason);
    channelName = channelName.substr(1);
    channelName = trim(channelName);
    userToKick = trim(userToKick);
    reason = trim(reason);

    std::string sender = channels[channelName].getNickname(fd);
    if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
        int userFd = channels[channelName].findUserFdForKickRegulars(userToKick);
        if (userFd != -1) {
            channels[channelName].ejectUserfromusers(userFd);
            channels[channelName].ejectUserfromivited(userToKick);
            std::string kickMessage = KICK_MESSAGE2(channelName, fd, userToKick, reason);
            smallBroadcastMsgForKick(sender, channelName, userToKick, reason);
            sendData(fd, kickMessage);
            sendData(userFd, kickMessage);
        } 
        else {
            std::string errorMessage = ERROR_MESSAGE3(sender, channelName, userToKick);
            sendData(fd, errorMessage);
        }
    }
    else {
        std::string errorMessage = ERROR_MESSAGE4(sender, channelName, userToKick);
        sendData(fd, errorMessage);
    }
}

void Server::processTopicCmd(Client& client, const std::string& command, int fd)
{
    std::string channelName, topic;
    std::istringstream iss(command.substr(6));
    iss >> channelName;
    std::getline(iss, topic);
    
    if (iss.fail()) {
        std::string errorMessage = "Error: You Just missing an argument(3)\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }
    
    channelName = channelName.substr(1);
    channelName = trim(channelName);
    topic = trim(topic);
    topic = topic.substr(1);
    
    std::string sender = channels[channelName].getNickname(fd);
    if ((channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) || !channels[channelName].isTopicRestricted()) {
        channels[channelName].setTopic(topic);
        std::string topicMessage = TOPIC_MESSAGE2(sender, channelName, topic);
        sendData(fd, topicMessage);
        smallbroadcastMessageforTopic(sender, channelName, topic);
    }
    else {
        std::string errorMessage = ERROR_MESSAGE6(sender, channelName);
        sendData(fd, errorMessage);
    }
}

void Server::processInviteCmd(Client& client, const std::string& command, int fd) {
    std::string channelName, nickname;
    std::istringstream iss(command.substr(7));
    iss >> nickname >> channelName;
    
    if (iss.fail()) {
        std::string errorMessage = "Error: You Just missing an argument(2)\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    channelName = trim(channelName);
    nickname = trim(nickname);
    channelName = channelName.substr(1);
    
    if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
        channels[channelName].addClientinveted(nickname, fd);
        handleInvitation(fd, nickname, channelName);
    }
    else {
        std::string errorMessage = ERROR_MESSAGE7(channelName, fd);
        sendData(fd, errorMessage);
    }
}

void Server::processBotCmd(Client& client, const std::string& command, int fd) {
    std::string start, end, guessed;
    std::istringstream iss(command.substr(4));
    iss >> start >> end >> guessed;
    std::string reme;
    
    if (iss.fail() || iss >> reme)
    {
        std::string errorMessage = "Error: Command take 3 parameters\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    start = trim(start);
    end = trim(end);
    guessed = trim(guessed);

    if (stringToInt(start) < stringToInt(end) && stringToInt(guessed) >= stringToInt(start) && stringToInt(guessed) <= stringToInt(end)) {
        int r = randomInRange(stringToInt(start), stringToInt(end));
        std::string random = intToString(r);
        if (random == guessed) {
            std::string congratsMsg = CONGRATS_MSG(guessed);
            sendData(fd, congratsMsg);
        }
        else {
            std::string guessAgain = GUESS_AGAIN(random);
            sendData(fd, guessAgain);
        }
    }
    else {
        std::string errorMessage = GUESS_ERROR();
        sendData(fd, errorMessage);
    }
}