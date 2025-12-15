NAME = ircserv

SRC = main.cpp ./src/Server.cpp ./src/Client.cpp ./src/Channel.cpp ./src/Authentication.cpp \
		./src/Broadcast.cpp ./src/Commands.cpp ./src/Helpers.cpp ./src/Modes.cpp

OBJ = ${SRC:.cpp=.o}

CXX = c++

CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -I ./inc

all : $(NAME)

$(NAME) : $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "\033[1;32m---- BUILT SUCCESSFULLY! ----\033[0m"

%.o: %.cpp
	@echo "\033[36mCompiling $<...\033[0m"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	@rm -rf *.o
	@rm -rf ./src/*.o
	@echo "\033[31mObject files cleaned.\033[0m"

fclean : clean
	@rm -rf $(NAME)
	@echo "\033[31mExecutable cleaned.\033[0m"

re : fclean all
