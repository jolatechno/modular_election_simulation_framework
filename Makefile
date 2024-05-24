all: test

par: test-par

all+par: all par

test:
	g++ -O3 test.cpp -o test.out

test-par:
	g++ -fopenmp -O3 test.cpp -o test-par.out