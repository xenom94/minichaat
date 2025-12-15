#include "Server.hpp"

void Server::processModeCmd(Client& client, const std::string& command, int fd) {
    std::string channelName, mode , nick;
    std::istringstream iss(command.substr(5));
    iss >> channelName >> mode >> nick;
    
    if (channelName[0] != '#') {
        client.clearCommand();
        return;
    }
    
    channelName = channelName.substr(1);
    channelName = trim(channelName);
    mode = trim(mode);
    size_t opt = mode.length() - 1;

    if (mode[opt] == 'o')
        handleOpPrivilege(nick, channelName, mode, fd);
    else if (mode[opt] == 't')
        handleTopicRestriction(nick, channelName, mode, fd);
    else if (mode[opt] == 'i')
        handleInviteOnly(nick, channelName, mode, fd);
    else if (mode[opt] == 'k')
        handleChannelKey(nick, channelName, mode, fd);
    else if (mode[opt] == 'l')
        handleChannelLimit(nick, channelName, mode, fd);
}

void Server::handleOpPrivilege(const std::string& nick, const std::string& channelName, const std::string& mode, int fd) {
    if (mode == "+o") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            channels[channelName].addOperator(nick, channels[channelName].getUserFd(nick));
            std::string modeChangeMessage = MODE_SET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE7(channelName, fd);
            sendData(fd, errorMessage);
        }
    }
    else if (mode == "-o") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            channels[channelName].removeOperator(nick);
            std::string modeChangeMessage = MODE_UNSET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
}

void Server::handleTopicRestriction(const std::string& nick, const std::string& channelName, const std::string& mode, int fd) {
    if (mode == "-t") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = TOPIC_CHANGE_MESSAGE(channelName, mode, fd);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setTopicRestricted(false);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
    else if (mode == "+t") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = TOPIC_CHANGE_MESSAGE(channelName, mode, fd);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setTopicRestricted(true);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
}

void Server::handleInviteOnly(const std::string& nick, const std::string& channelName, const std::string& mode, int fd) {
    if (mode == "+i") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = MODE_SET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setInviteOnly(true);
        }
        else if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd) == false) {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
        
    }
    else if (mode == "-i") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = MODE_UNSET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setInviteOnly(false);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
}

void Server::handleChannelKey(std::string& nick, const std::string& channelName, const std::string& mode, int fd) {
    if (mode == "-k") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = MODE_UNSET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setHasPassword(false);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
    else if (mode == "+k") {
        nick = trim(nick);
        channels[channelName].setPass(nick);
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = MODE_SET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setHasPassword(true);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
}

void Server::handleChannelLimit(const std::string& nick, const std::string& channelName, const std::string& mode, int fd) {
    if (mode == "+l") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            int limit = stringToInt(nick);
            channels[channelName].setlimitchannel(limit);
            std::string modeChangeMessage = MODE_SET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setHasLimit(true);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
    else if (mode == "-l") {
        if (channels.find(channelName) != channels.end() && channels[channelName].isOperator(fd)) {
            std::string modeChangeMessage = MODE_UNSET_MESSAGE(channelName, mode, fd, nick);
            sendData(fd, modeChangeMessage);
            smallbroadcastMOOD(channels[channelName].getNickname(fd), channelName, mode, nick);
            channels[channelName].setHasLimit(false);
        }
        else {
            std::string errorMessage = ERROR_MESSAGE5();
            sendData(fd, errorMessage);
        }
    }
}
