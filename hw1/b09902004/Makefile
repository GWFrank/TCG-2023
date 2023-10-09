CPPFLAGS = -std=c++11 -O2 -Wall -Wextra
DEBUGFLAGS = -std=c++11 -Og -Wall -Wextra

all: solve

debug: solve_debug

solve: src/solve.cpp src/ewn.cpp
	g++ ${CPPFLAGS} $^ -o $@

solve_debug: src/solve.cpp src/ewn.cpp
	g++ ${DEBUGFLAGS} $^ -o $@

clean:
	rm -f solve solve_debug
