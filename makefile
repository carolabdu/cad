# Compilador C padrão
CC = gcc
# Compilador MPI (usado para códigos MPI)
MPICC = mpicc
# Flags de compilação comuns: -O2 para otimização, -Wall para ativar todos os avisos
CFLAGS = -O2 -Wall
# Flags de linkagem para OpenMP
LDFLAGS_OPENMP = -fopenmp

# Regra 'all': compila todos os executáveis
all: odd_even_serial odd_even_openmp odd_even_mpi

# Regra para compilar o código serial
odd_even_serial: odd_even_serial.c
	$(CC) $(CFLAGS) -o $@ $<

# Regra para compilar o código OpenMP
# Requer a flag -fopenmp para habilitar as diretivas OpenMP
odd_even_openmp: odd_even_openmp.c
	$(CC) $(CFLAGS) $(LDFLAGS_OPENMP) -o $@ $<

# Regra para compilar o código MPI
# Usa o compilador MPI (mpicc) que já inclui as bibliotecas e flags necessárias
odd_even_mpi: odd_even_mpi.c
	$(MPICC) $(CFLAGS) -o $@ $<

# Regra 'clean': remove todos os executáveis e arquivos temporários gerados
clean:
	rm -f odd_even_serial odd_even_openmp odd_even_mpi *.o

test: all
	./odd_even_serial 100000
	./odd_even_openmp 100000 1 static
	./odd_even_openmp 100000 2 static
	./odd_even_openmp 100000 2 dynamic
	./odd_even_openmp 100000 2 guided
	./odd_even_openmp 100000 4 static
	./odd_even_openmp 100000 4 dynamic
	./odd_even_openmp 100000 4 guided
	./odd_even_openmp 100000 8 static
	./odd_even_openmp 100000 8 dynamic
	./odd_even_openmp 100000 8 guided
	mpirun -np 1 ./odd_even_mpi 100000
	mpirun -np 2 ./odd_even_mpi 100000
	mpirun -np 4 ./odd_even_mpi 100000