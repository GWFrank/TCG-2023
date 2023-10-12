CPPFLAGS = -std=c++11 -O3 -Wall -Wextra
DEBUGFLAGS = -std=c++11 -Og -Wall -Wextra

all: solve

profile: solve_prof

debug: solve_debug

solve: src/solve.cpp src/ewn.cpp
	g++ ${CPPFLAGS} $^ -o $@

solve_debug: src/solve.cpp src/ewn.cpp
	g++ ${DEBUGFLAGS} $^ -o $@

solve_prof: src/solve.cpp src/ewn.cpp
	g++ -pg ${CPPFLAGS} $^ -o $@

clean:
	rm -f solve solve_debug solve_prof
