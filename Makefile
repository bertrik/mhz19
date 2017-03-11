CXXFLAGS = -W -Wall -O2

all: mhz19_test

clean:
	rm -f mhz19_test

test: mhz19_test
	./mhz19_test

