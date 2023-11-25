CXXFLAGS = -std=c++20 -O2 -Wall -Wextra
CXXFLAGS_DEBUG = -std=c++20 -Og -Wall -Wextra
RELEASE = -D NDEBUG

all: agent_release
debug: agent_debug
release: agent_release

agent_debug: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS_DEBUG) src/agent.cpp src/ewn.cpp -o agent_debug

agent_release: src/agent.cpp src/ewn.cpp src/ewn.hpp
	g++ $(CXXFLAGS) $(RELEASE) src/agent.cpp src/ewn.cpp -o agent

clean:
	rm -f agent agent_debug
