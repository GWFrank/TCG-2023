CXXFLAGS = -std=c++11 -O2 -Wall -Wextra
EASY = -D EASY
NORMAL = -D NORMAL
HARD = -D HARD

all: easy normal hard

easy: src/agent.cpp src/ewn.cpp src/heuristic.cpp
	g++ $(CXXFLAGS) $(EASY) $^ -o $@

normal: src/agent.cpp src/ewn.cpp src/heuristic.cpp
	g++ $(CXXFLAGS) $(NORMAL) $^ -o $@

hard: src/agent.cpp src/ewn.cpp src/heuristic.cpp
	g++ $(CXXFLAGS) $(HARD) $^ -o $@

clean:
	rm -f easy normal hard
