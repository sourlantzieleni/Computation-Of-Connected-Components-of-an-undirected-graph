SRC =  colouringCC.c mmio.c converter.c
TARGET = seirial

compile:
	gcc -O3 colouringCC.c mmio.c converter.c -o seirial
	gcc -O3 -fopenmp colouringCC_OpenMP.c mmio.c converter.c -o parallel_MP
	clang -O3 -fopencilk colouringCC_OpenCilk.c mmio.c converter.c -o parallel_Cilk
	gcc -O3 -pthread colouringCC_pthreads.c mmio.c converter.c -o parallel_pthreads

run: compile
	@echo "===== SERIAL ====="
	./seirial ../data/mycielskian5.mtx

	@echo "===== OPENMP ====="
	./parallel_MP ../data/mycielskian5.mtx

	@echo "===== OPENCILK ====="
	CILK_NWORKERS=16 ./parallel_Cilk ../data/mycielskian5.mtx

	@echo "===== PTHREADS ====="
	./parallel_pthreads ../data/mycielskian5.mtx

clean:
	rm -f seirial
	rm -f parallel_MP
	rm -f parallel_Cilk
	rm -f parallel_pthreads
