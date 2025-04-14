# Compiler settings
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = ircserv

# Source files
SRCS = src/Channel.cpp \
		src/Client.cpp \
		src/main.cpp \
		src/parsing.cpp \
		src/Server.cpp \
		src/commands/bet.cpp \
		src/commands/dcc.cpp \
		src/commands/invite.cpp \
		src/commands/join.cpp \
		src/commands/login.cpp \
		src/commands/mode.cpp \
		src/commands/privmsg.cpp \
		src/commands/topic.cpp

OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(NAME)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linking
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Clean object files
clean:
	rm -f $(OBJS)

# Clean everything
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

# Prevent conflicts with files named like our targets
.PHONY: all clean fclean re
