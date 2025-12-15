#ifndef MACROS_HPP
#define MACROS_HPP

#define OO1 ":irc.server 001 " + nickname + " :Welcome to the MiniChat Network, " + nickname + "\r\n"
#define OO2 ":irc.server 002 " + nickname + " :Your host is MiniChat, running version 4.5\r\n"
#define OO3 ":irc.server 003 " + nickname + " :This server was created " + formatCreationTime() + "\r\n"
#define OO4 ":irc.server 004 " + nickname + " MiniChat MiniChat(enterprise)-2.3(12)-netty(5.4c)-proxy(0.9) MiniChat\r\n"

#define JOIN_MESSAGE(nickname, channelName) (":" + nickname + " JOIN #" + channelName + "\r\n")
#define MODE_MESSAGE(channelName) (":irc.server MODE #" + channelName + " +nt\r\n")
#define NAMES_MESSAGE(nickname, channelName) (":irc.server 353 " + nickname + " = #" + channelName + " :@" + nickname + "\r\n")
#define NAMES_MESSAGE2(nickname, channelName) (":irc.server 353 " + nickname + " @ #" + channelName + " :")
#define END_OF_NAMES_MESSAGE(nickname, channelName) (":irc.server 366 " + nickname + " #" + channelName + " :End of /NAMES list.\r\n")
#define CHANNEL_MESSAGE(channelName, creationTimeMessage) (":irc.server 354 " + channelName + " " + creationTimeMessage + "\r\n")
#define TOPIC_MESSAGE(nickname, channelName, topic) (":irc.server 332 " + nickname + " #" + channelName + " :" + topic + " https://irc.com\r\n")
#define TOPIC_MESSAGE2(nicknamesender, channelname, topic) (":" + nicknamesender + " TOPIC #" + channelname + " :" + topic + "\r\n")

#define PRIVATE_MESSAGE(senderFd, recipient, message) (":" + nicknames[senderFd] + " PRIVMSG " + recipient + " :" + message + "\r\n")
#define ERROR_MESSAGE(senderFd, recipient) (":server.host NOTICE " + nicknames[senderFd] + " :Error: User '" + recipient + "' not found or offline\r\n")
#define INVITE_MESSAGE(senderFd, recipient, channelName) (":" + nicknames[senderFd] + " INVITE " + recipient + " :#" + channelName + "\r\n")
#define KICK_MESSAGE(nicknamesender, channelname, usertokick, reason) (":" + nicknamesender + " KICK #" + channelname + " " + usertokick + " :" + reason + "\r\n")
#define KICK_MESSAGE2(channelName, fd, userToKick, reason) (":" + channels[channelName].getNickname(fd) + " KICK #" + channelName + " " + userToKick + " :" + reason + "\r\n")

#define MODE_CHANGE_MESSAGE(channelname, mode, sender, receiver) \
    (mode == "+t" ? MODE_CHANGE_T(channelname, sender) : \
    (mode == "-t" ? MODE_CHANGE_MINUS_T(channelname, sender) : \
    (mode == "+o" ? MODE_CHANGE_O(channelname, sender, mode, receiver) : \
    (mode == "-o" ? MODE_CHANGE_MINUS_O(channelname, sender, mode, receiver) : \
    (mode == "+i" ? MODE_CHANGE_IK(channelname, sender, mode) : \
    (mode == "-i" ? MODE_CHANGE_IK(channelname, sender, mode) : \
    (mode == "+k" ? MODE_CHANGE_IK(channelname, sender, mode) : \
    (mode == "-k" ? MODE_CHANGE_IK(channelname, sender, mode) : \
    (mode == "+l" ? MODE_CHANGE_L(channelname, sender, mode) : \
    (mode == "-l" ? MODE_CHANGE_L(channelname, sender, mode) : ""))))))))))

#define MODE_CHANGE_T(channelname, sender) (MODE_CHANGE_MINUS_T(channelname, sender))
#define MODE_CHANGE_MINUS_T(channelname, sender) (":" + sender + " MODE #" + channelname + " -t\r\n")
#define MODE_CHANGE_O(channelname, sender, mode, receiver) (":" + sender + " MODE #" + channelname + " " + mode + " " + receiver + "\r\n")
#define MODE_CHANGE_MINUS_O(channelname, sender, mode, receiver) (":" + sender + " MODE #" + channelname + " " + mode + " " + receiver + "\r\n")
#define MODE_CHANGE_IK(channelname, sender, mode) (":" + sender + " MODE #" + channelname + " " + mode + "\r\n")
#define MODE_CHANGE_L(channelname, sender, mode) (":" + sender + " MODE #" + channelname + " " + mode + "\r\n")

#define ERROR_MESSAGE2(nick) (":server.host NOTICE " + nick + what)
#define ERROR_INVITEONLYCHAN(nick, channelName) (":server.host 473 " + nick + " #" + channelName + " :Cannot join channel (+i)\r\n")
#define ERROR_NOSUCHCHANNEL(nick, channelName) (std::string(":server.host 403 ") + nick + " #" + channelName + " :No such channel\r\n")
#define ERROR_NOTONCHANNEL(nick, channelName) (std::string(":server.host 442 ") + nick + " #" + channelName + " :You're not on that channel\r\n")
#define ERROR_NEEDMOREPARAMS(nick, command) (std::string(":server.host 461 ") + nick + " " + command + " :Not enough parameters\r\n")
#define ERROR_CHANNELISFULL(nick, channelName) (std::string(":server.host 471 ") + nick + " #" + channelName + " :Cannot join channel (+l)\r\n")
#define ERROR_BADCHANNELKEY(nick, channelName) (std::string(":server.host 475 ") + nick + " #" + channelName + " :Cannot join channel (+k)\r\n")
#define ERROR_CHANOPRIVSNEEDED(nick, channelName) (std::string(":server.host 482 ") + nick + " #" + channelName + " :You're not channel operator\r\n")
#define ERROR_NONICKNAMEGIVEN(nick) (std::string(":server.host 431 ") + nick + " :No nickname given\r\n")
#define ERROR_ERRONEUSNICKNAME(nick, badnick) (std::string(":server.host 432 ") + nick + " " + badnick + " :Erroneous nickname\r\n")
#define ERROR_NICKNAMEINUSE(nick, badnick) (std::string(":server.host 433 ") + nick + " " + badnick + " :Nickname is already in use\r\n")
#define ERROR_USERNOTINCHANNEL(nick, user, channel) (std::string(":server.host 441 ") + nick + " " + user + " #" + channel + " :They aren't on that channel\r\n")
#define ERROR_MESSAGE3(sender, channelName, userToKick) (":" + sender + " PRIVMSG #" + channelName + " :Error: the user : " + userToKick + " is not found or offline.\r\n")
#define ERROR_MESSAGE4(sender, channelName, userToKick) (":" + sender + " PRIVMSG #" + channelName + " :Error1: You are not authorized to execute this command " + userToKick + "\r\n")
#define ERROR_MESSAGE5() (":" + channels[channelName].getNickname(fd) + " PRIVMSG #" + channelName + " :Error: You are not authorized to execute this command " + "\r\n")
#define ERROR_MESSAGE6(sender, channelName) (":" + sender + " PRIVMSG #" + channelName + " :Error2: You are not authorized to execute this command\r\n")
#define ERROR_MESSAGE7(channelName, fd) (":" + channels[channelName].getNickname(fd) + " PRIVMSG #" + channelName + " :Error: You are not authorized to execute this command\r\n")

#define CONGRATS_MSG(guessed) (":server.host PRIVMSG #:Congratulations ,you have guessed the number : " + guessed + " correctly\n")
#define GUESS_AGAIN(random) (":server.host PRIVMSG #:Sorry ,the correct one is : " + random + "\n")
#define GUESS_ERROR() (":server.host PRIVMSG # :Error: Invalid range or guess\n")
#define MODE_SET_MESSAGE(channelName, mode, fd, nick) (":" + channels[channelName].getNickname(fd) + " MODE #" + channelName + " " + mode + " " + nick + "\r\n")
#define MODE_UNSET_MESSAGE(channelName, mode, fd, nick) (":" + channels[channelName].getNickname(fd) + " MODE #" + channelName + " " + mode + " " + nick + "\r\n")
#define TOPIC_CHANGE_MESSAGE(channelName, mode, fd) (":" + channels[channelName].getNickname(fd) + " MODE #" + channelName + " " + mode + "\r\n")

#define BUFFER_SIZE 1024
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

#define MINICHAT  "\n$$\\      $$\\ $$\\           $$\\  $$$$$$\\  $$\\                  $$\\     \n" \
                  "$$$\\    $$$ |\\__|          \\__|$$  __$$\\ $$ |                 $$ |    \n" \
                  "$$$$\\  $$$$ |$$\\ $$$$$$$\\  $$\\ $$ /  \\__|$$$$$$$\\   $$$$$$\\ $$$$$$\\   \n" \
                  "$$\\$$\\$$ $$ |$$ |$$  __$$\\ $$ |$$ |      $$  __$$\\  \\____$$\\_$$  _|  \n" \
                  "$$ \\$$$  $$ |$$ |$$ |  $$ |$$ |$$ |      $$ |  $$ | $$$$$$$ | $$ |    \n" \
                  "$$ |\\$  /$$ |$$ |$$ |  $$ |$$ |$$ |  $$\\ $$ |  $$ |$$  __$$ | $$ |$$\\ \n" \
                  "$$ | \\_/ $$ |$$ |$$ |  $$ |$$ |\\$$$$$$  |$$ |  $$ |\\$$$$$$$ | \\$$$$  |\n" \
                  "\\__|     \\__|\\__|\\__|  \\__|\\__| \\______/ \\__|  \\__| \\_______|  \\____/ \n\n"

#endif