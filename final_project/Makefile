CXXFLAGS = -std=c++20 -Wall -Wextra
CXXFLAGS_DEBUG = -std=c++20 -Wall -Wextra
RELEASE = -O3 -funroll-loops -D NDEBUG
DEBUG = -Og
RANDOM = -D RANDOM_MOVE

all: release
debug: agent_debug
release: agent_release
random: agent_random

agent_release: src/main.cpp src/agent.cpp src/agent.h
	g++ $(CXXFLAGS) $(RELEASE) src/main.cpp src/agent.cpp -o agent

agent_debug: src/main.cpp src/agent.cpp src/agent.h
	g++ $(CXXFLAGS_DEBUG) $(DEBUG) src/main.cpp src/agent.cpp -o agent_debug

agent_random: src/main.cpp src/agent.cpp src/agent.h
	g++ $(CXXFLAGS_DEBUG) $(DEBUG) $(RANDOM) src/main.cpp src/agent.cpp -o agent_random


clean:
	rm -f agent agent_debug agent_random
