CXXFLAGS=-std=c++20 -Wall -Wextra
CXXFLAGS_DEBUG=-std=c++20 -Wall -Wextra
RELEASE=-O3 -funroll-loops -D NDEBUG
DEBUG=-Og
RANDOM=-D RANDOM_MOVE

SOURCES=src/main.cpp src/agent.cpp src/agent.h src/precompute.h

all: agent
debug: agent_debug
random: agent_random

agent: $(SOURCES)
	g++ $(CXXFLAGS) $(RELEASE) src/main.cpp src/agent.cpp -o agent

agent_debug: $(SOURCES)
	g++ $(CXXFLAGS_DEBUG) $(DEBUG) src/main.cpp src/agent.cpp -o agent_debug

agent_random: $(SOURCES)
	g++ $(CXXFLAGS_DEBUG) $(RELEASE) $(RANDOM) src/main.cpp src/agent.cpp -o agent_random


clean:
	rm -f agent agent_debug agent_random
