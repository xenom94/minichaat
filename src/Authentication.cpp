#include "Server.hpp"

void Server::processPassword(Client& client, const std::string& command, int fd) {
    std::string passwordLine = command.substr(command.find(" ") + 1);
    passwordLine = trim(passwordLine);

    if (isValidPassword(passwordLine)) {
        passwordLine = passwordLine.substr(1, passwordLine.size() - 2);
    }

    if (passwordLine.empty()) {
        std::string errorMessage = "Error: Password cannot be empty\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    std::string serverPassword = getPassword();
    if (serverPassword != passwordLine) {
        std::string errorMessage = "Error: Incorrect Password\n";
        sendData(fd, errorMessage);
    } else {
        std::string confirmation = "Please Enter Your Nickname : \n";
        sendData(fd, confirmation);
        client.setAuthentication(1);
    }
}

void Server::processNickCmd(Client& client, const std::string& command, int fd) {
    std::string cmd, nick;
    std::istringstream iss(command);
    iss >> cmd >> nick;
    nick = trim(nick);

    // Check if there are more tokens after the nickname
    std::string remaining;
    if (iss >> remaining) {
        std::string errorMessage = "Error: Command requires only 1 parameter\n";
            sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    // Validate the nickname
    if (dontputthesamenick(nick)) {
        std::string confirmation = "Please Use a Different Nickname : \n";
        sendData(fd, confirmation);
    }
    else {
        setNickname(fd, nick);
        for (size_t i = 0; i < _clients.size(); ++i)
        {
            if (_clients[i].getFd() == fd) {
                _clients[i].setNick(nick);
                std::cout << "Nickname set for client " << fd << ": " << nick << std::endl;
                break;
            }
        }
        std::string confirmation = "Please Enter Your Username : \n";
        sendData(fd, confirmation);
        client.setAuthentication(2);
    }
}

void Server::processUserCmd(Client& client, const std::string& command, int fd)
{
    std::istringstream iss(command);
    std::string cmd, username, dontworry, dontworry1, realname, nickname;
    iss >> cmd >> username >> dontworry >> dontworry1 >> realname;
    username = trim(username);
    dontworry = trim(dontworry);
    dontworry1 = trim(dontworry1);
    realname = trim(realname);

    std::string reme;
    if (iss.fail() || iss >> reme) {
        std::string errorMessage = "Error: Command requires four parameters\n";
        sendData(fd, errorMessage);
        client.clearCommand();
        return;
    }

    if (dontputthesameusername(username) == true) {
        std::string confirmation = "Please Use a Different username : \n";
        sendData(fd, confirmation);
    }
    else {
        setUsernames(fd, username);
        for (size_t i = 0; i < _clients.size(); ++i) {
            if (_clients[i].getFd() == fd) {
                _clients[i].setUser(username);
                _clients[i].setName(realname);
                _clients[i].setAuthentication(3);
                nickname = _clients[i].getNick();
                break;
            }
        }
        welcome(nickname, fd);
    }
}

void Server::welcome(const std::string& nickname, int fd){
    std::string one = OO1;
    std::string two = OO2;
    std::string tre = OO3;
    std::string foor = OO4;
    sendData(fd, one);
    sendData(fd, two);
    sendData(fd, tre);
    sendData(fd, foor);
}
