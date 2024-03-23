COMP_MPI = mpic++ -fpic -std=c++20 -Wall -Wextra -O3 -msse2 -mavx -lmpi
MPIRUN = mpirun

#############################################################################################################################

seqpi: SeqPi.cpp
	g++ -fopenmp -std=c++20 -Wall -Wextra -O3 -msse2 -mavx SeqPi.cpp -o seqpi

spi: seqpi
	./seqpi 1e9

ppi: Pi.cpp
	LD_LIBRARY_PATH=""
	PATH=""
	${COMP_MPI} Pi.cpp -o ppi

pi: ppi
	${MPIRUN} -np 6 ./ppi 1e9

tpi: ppi
	python3 ./test_performance.py 6 1 2 3 4 5 6 3 ./ppi 1e9

#############################################################################################################################

sync_time: sync_time.cpp
	LD_LIBRARY_PATH=""
	${COMP_MPI} sync_time.cpp -o sync_time

st: sync_time
	${MPIRUN} -np 2 ./sync_time

#############################################################################################################################

COMP_TRANSFER = mpic++ -lmpi 
ARGS = -g3 -fpic -std=c++20 -Wall -Wextra -O3 -msse2 -mavx

obj/mesh.o: transfer/mesh.cpp transfer/mesh.h transfer/constant.h
	${COMP_TRANSFER} ${ARGS} -c transfer/mesh.cpp -o obj/mesh.o

obj/domain.o: transfer/domain.cpp transfer/domain.h transfer/constant.h
	${COMP_TRANSFER} ${ARGS} -c transfer/domain.cpp -o obj/domain.o

obj/transfer.o: transfer/transfer.cpp transfer/constant.h
	${COMP_TRANSFER} ${ARGS} -c transfer/transfer.cpp -o obj/transfer.o

obj/transfer_seq.o: transfer/transfer_seq.cpp transfer/constant.h
	${COMP_TRANSFER} ${ARGS} -c transfer/transfer_seq.cpp -o obj/transfer_seq.o

obj/functions.o: transfer/functions.cpp
	${COMP_TRANSFER} ${ARGS} -c transfer/functions.cpp -o obj/functions.o

tr: obj obj/transfer.o obj/mesh.o obj/domain.o obj/functions.o
	${COMP_TRANSFER} ${ARGS} obj/transfer.o obj/mesh.o obj/domain.o obj/functions.o -o tr

tr_seq: obj obj/transfer_seq.o obj/mesh.o obj/domain.o obj/functions.o
	${COMP_TRANSFER} ${ARGS} obj/transfer_seq.o obj/mesh.o obj/domain.o obj/functions.o -o tr_seq

str: tr tr_seq
	./tr

obj:
	mkdir -p obj

ttr: tr
	python3 ./test_transfer.py 4 1 2 3 4 3 ./tr
#	python3 ./test_transfer.py 6 1 2 3 4 5 6 1 ./tr

#############################################################################################################################

.PHONY: spi pi st tpi st t str run_tr