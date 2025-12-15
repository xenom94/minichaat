#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <climits>

// Port validation constants
#define MIN_VALID_PORT 1024
#define MAX_VALID_PORT 65535
// Maximum length of valid port string (5 digits for max port 65535)
#define MAX_PORT_LENGTH 5

bool valid_port(const std::string& port, int& port_num)
{
    if (port.length() == 0 || port.length() > MAX_PORT_LENGTH)
        return false;
    for (size_t i = 0; i < port.length(); ++i)
    {
        if (!isdigit(port[i]))
            return false;
    }
    
    // Use strtol for safer conversion with overflow detection
    char* endptr;
    errno = 0;
    long port_long = std::strtol(port.c_str(), &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || *endptr != '\0' || port_long < MIN_VALID_PORT || port_long > MAX_VALID_PORT)
        return false;
    
    port_num = static_cast<int>(port_long);
    return true;
}

int main(int ac, char** av)
{
    if (ac != 3)
    {
        std::cerr << "usage: " << av[0] << " <port> <password>" << std::endl;
        std::cerr << "example: " << av[0] << " 6667 mypassword" << std::endl;
        return EXIT_FAILURE;
    }

    std::string password = av[2];
    int port;
    if (valid_port(std::string(av[1]), port) == false)
    {
        std::cerr << "Error: Invalid port number." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        Server server(port, password);
        server.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << RED << ">>> SERVER CLOSED <<<" << RESET << std::endl;
    return EXIT_SUCCESS;
}
