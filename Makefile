# Compiler settings
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
NAME = ircserv

# Source files
SRCS = $(wildcard src/*.cpp) $(wildcard src/commands/*.cpp)
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(NAME)

# Linking
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
