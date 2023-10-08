CFLAGS = -std=c++11 -O2 -Wall -Wextra

all:
	@echo "usage: make verifier"

verifier: src/verifier.cpp src/ewn.cpp
	g++ ${CFLAGS} $^ -o $@

clean:
	rm -f verifier
