int: main.cpp
	g++ -O3 -std=c++17 main.cpp -o int -lpthread

t1: int
	./int 1 1e-6 1 1e-13 1 10000

td1: int
	./int 1 1e-6 1 1e-10 1 10000

td4: int
	./int 4 1e-6 1 1e-10 1 10000

t2: int
	./int 2 1e-6 1 1e-13 1 10000

t3: int
	./int 3 1e-6 1 1e-13 1 10000

t4: int
	./int 4 1e-6 1 1e-13 1 10000

t5: int
	./int 5 1e-6 1 1e-13 1 10000

t6: int
	./int 6 1e-6 1 1e-13 1 10000

test4: int
	python3 ./test_int.py 4 1 2 3 4 1 ./int 1e-6 1 1e-13 1 10000

test6: int
	python3 ./test_int.py 6 1 2 3 4 5 6 1 ./int 1e-6 1 1e-13 1 10000