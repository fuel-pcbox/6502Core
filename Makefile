
CPP=g++
CFLAGS=-O3 -g -std=c++11

run: test.run
	./test.run

default: test.run

m6502.o: m6502.cpp
	$(CPP) $(CFLAGS) -c m6502.cpp

main.o: main.cpp
	$(CPP) $(CFLAGS) -c main.cpp


test.run: m6502.o main.o
	$(CPP) $(CFLAGS) $^ -o test.run

clean:
	rm -f *.o *~ test.run
