#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <sstream>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include <cstring>
#include <map>
#include <string>

class Channel {
private:
    std::string Channelname;
    std::string topic;
    std::string key;
    std::string pass;
    int limit;
    bool topicRestricted;      // replaces issettop
    bool inviteOnly;           // replaces isinvited
    bool hasPassword;          // replaces itHasPass
    bool hasLimit;             // replaces channelLimit
    int currentUserCount;      // replaces limitchannelforincrement
    std::map<std::string, int> userFdMap; 
    std::map<std::string, int> invitedUsers;
    std::map<std::string, int>  operators;

public:

    Channel();
    Channel(const std::string& name);
    ~Channel();
    void setTopic(const std::string& newTopic);
    std::string getTopic() const;
    void setlimitchannel(int value);
    int getChannelLimit();


    void addClient(const std::string& client, int fd);
    void addClientinveted(const std::string& client, int fd);
    void addOperator(const std::string& operatorName, int fd);
    int getUserFd(const std::string& username) const;
    bool isUserInChannel(const std::string& nickname) const;
    std::vector<std::string> getClients() const;
    std::string getNickname(int fd) const;
    bool isOperator(int fd);
    bool isInvited(std::string nickname);
    int findUserFdForKickRegulars(const std::string& username);
    void ejectUserfromusers(int fd);
    void ejectUserfromivited(std::string nickname);
    std::string getOperatorNickname(int fd) const;
    void removeOperator(const std::string& operatorName );
    

    void setPass(const std::string &Newpass);

    std::string getPass();
    std::map<std::string, int>& getUserFdMap();
    std::map<std::string, int>& invitedUserss();
    std::map<std::string, int>& getOperators();
    
    // Mode getters/setters
    bool isTopicRestricted() const;
    void setTopicRestricted(bool value);
    
    bool isInviteOnly() const;
    void setInviteOnly(bool value);
    
    bool hasPasswordSet() const;
    void setHasPassword(bool value);
    
    bool hasUserLimit() const;
    void setHasLimit(bool value);
    
    int getCurrentUserCount() const;
    void incrementUserCount();
    void decrementUserCount();
    void resetUserCount();
};

#endif