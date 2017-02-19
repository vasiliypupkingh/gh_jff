CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -Werror

all: shell_imit

shell_imit: shell.cpp
	$(CXX) $(CXXFLAGS) -o shell_imit shell.cpp

.PHONY: clean

clean:
	rm -f shell_imit

