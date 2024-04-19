int: main.cpp
	g++ -O2 -std=c++17 main.cpp -o int -lpthread

t1: int
	./int 1 1e-6 1 1e-13 1 10000

t2: int
	./int 2 1e-6 1 1e-13 1 10000

t3: int
	./int 3 1e-6 1 1e-13 1 10000

t4: int
	./int 4 1e-6 1 1e-13 1 10000

test: int
	python3 ./test_int.py 4 1 2 3 4 1 ./int 1e-6 1 1e-13 1 10000