CXXFLAGS = -std=c++20 -Wall -Wextra
CXXFLAGS_DEBUG = -std=c++20 -Wall -Wextra
DEBUG = -Og
RELEASE = -O3 -funroll-loops -D NDEBUG
MCS = -D MCS_ALGO
TEMPERATURE = -D TEMPERATURE

all: agent_release
debug: agent_debug
release: agent_release
mcs: agent_mcs
temp: agent_temperature

agent_debug: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS_DEBUG) $(DEBUG) src/agent.cpp src/ewn.cpp -o agent_debug

agent_release: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS) $(RELEASE) src/agent.cpp src/ewn.cpp -o agent

agent_mcs: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS) $(RELEASE) $(MCS) src/agent.cpp src/ewn.cpp -o agent_mcs

agent_temperature: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS) $(RELEASE) $(TEMPERATURE) src/agent.cpp src/ewn.cpp -o agent_temperature

clean:
	rm -f agent agent_debug agent_mcs
