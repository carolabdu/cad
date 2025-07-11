CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall
OPENMP_FLAGS = -fopenmp

# Diretório para salvar os resultados
RESULTS_DIR = results
LOG_FILE_PREFIX = odd_even_sort_

.PHONY: all clean test_serial test_openmp test_mpi run_all_tests setup_results_dir

all: odd_even_serial odd_even_openmp odd_even_mpi

# Regra para compilar a versão serial
odd_even_serial: odd_even_serial.c
	$(CC) $(CFLAGS) -o odd_even_serial odd_even_serial.c

# Regra para compilar a versão OpenMP
odd_even_openmp: odd_even_openmp.c
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) -o odd_even_openmp odd_even_openmp.c

# Regra para compilar a versão MPI
odd_even_mpi: odd_even_mpi.c
	$(MPICC) $(CFLAGS) -o odd_even_mpi odd_even_mpi.c

# Limpa os executáveis e o diretório de resultados
clean:
	rm -f odd_even_serial odd_even_openmp odd_even_mpi
	rm -rf $(RESULTS_DIR)

# Cria o diretório de resultados se não existir
setup_results_dir:
	@mkdir -p $(RESULTS_DIR)

# Tamanhos de array para os testes
ARRAY_SIZES = 1000 5000 10000 50000 100000

# Números de threads/processos para os testes
NUM_PROCS = 1 2 4 8

# Número de repetições por teste
NUM_REPETITIONS = 3

# Teste para a versão serial
test_serial: setup_results_dir odd_even_serial
	@echo "--- Executando testes para a versão Serial ---"
	@for size in $(ARRAY_SIZES); do \
		LOG_FILE=$(RESULTS_DIR)/$(LOG_FILE_PREFIX)serial_$${size}.log; \
		echo "Executando Serial (N=$${size})... resultados em $${LOG_FILE}"; \
		echo "Tamanho do Array: $${size}" > $${LOG_FILE}; \
		for i in $(seq 1 $(NUM_REPETITIONS)); do \
			echo "Repetição $$i:" >> $${LOG_FILE}; \
			/usr/bin/time -f "Tempo real: %e segundos" ./odd_even_serial $${size} >> $${LOG_FILE} 2>&1; \
		done; \
	done; \
	echo "Testes seriais concluídos. Verifique a pasta '$(RESULTS_DIR)/'."

# Teste para a versão OpenMP
test_openmp: setup_results_dir odd_even_openmp
	@echo "--- Executando testes para a versão OpenMP ---"
	@for size in $(ARRAY_SIZES); do \
		for threads in $(NUM_PROCS); do \
			for policy_code in 0 1 2; do \
				if [ $$policy_code -eq 0 ]; then POLICY="static"; fi; \
				if [ $$policy_code -eq 1 ]; then POLICY="dynamic"; fi; \
				if [ $$policy_code -eq 2 ]; then POLICY="guided"; fi; \
				LOG_FILE=$(RESULTS_DIR)/$(LOG_FILE_PREFIX)openmp_N$${size}_T$${threads}_$$POLICY.log; \
				echo "Executando OpenMP (N=$${size}, T=$${threads}, Pol=$${POLICY})... resultados em $${LOG_FILE}"; \
				echo "Tamanho do Array: $${size}, Threads: $${threads}, Política: $${POLICY}" > $${LOG_FILE}; \
				for i in $(seq 1 $(NUM_REPETITIONS)); do \
					echo "Repetição $$i:" >> $${LOG_FILE}; \
					/usr/bin/time -f "Tempo real: %e segundos" ./odd_even_openmp $${size} $${threads} $$policy_code >> $${LOG_FILE} 2>&1; \
				done; \
			done; \
		done; \
	done; \
	echo "Testes OpenMP concluídos. Verifique a pasta '$(RESULTS_DIR)/'."

# Teste para a versão MPI (requer OpenMPI ou MPICH instalado e configurado)
test_mpi: setup_results_dir odd_even_mpi
	@echo "--- Executando testes para a versão MPI ---"
	@for size in $(ARRAY_SIZES); do \
		for procs in $(NUM_PROCS); do \
			# Pula se o número de processos for 1 e o size não for um múltiplo de 1, etc.
			# Ou se procs > size (não faz sentido ter mais processos que elementos no array)
			if [ $$procs -gt $$size ]; then continue; fi; \
			if [ $$(( $${size} % $${procs} )) -ne 0 ]; then \
				echo "Aviso: Tamanho do array $${size} não é divisível por $$procs processos para MPI. Pulando." >> $(RESULTS_DIR)/warnings.log; \
				continue; \
			fi; \
			LOG_FILE=$(RESULTS_DIR)/$(LOG_FILE_PREFIX)mpi_N$${size}_P$${procs}.log; \
			echo "Executando MPI (N=$${size}, P=$${procs})... resultados em $${LOG_FILE}"; \
			echo "Tamanho do Array: $${size}, Processos: $${procs}" > $${LOG_FILE}; \
			for i in $(seq 1 $(NUM_REPETITIONS)); do \
				echo "Repetição $$i:" >> $${LOG_FILE}; \
				# Usamos 'mpirun' para executar o programa MPI
				# 'time' pode ser tricky com mpirun, a medição de tempo interna do programa é mais confiável.
				/usr/bin/time -f "Tempo real (comando time): %e segundos" mpirun -np $${procs} ./odd_even_mpi $${size} >> $${LOG_FILE} 2>&1; \
			done; \
		done; \
	done; \
	echo "Testes MPI concluídos. Verifique a pasta '$(RESULTS_DIR)/'."

# Regra para rodar todos os testes
run_all_tests: test_serial test_openmp test_mpi
	@echo "\n--- Todos os testes concluídos! ---"
	@echo "Os resultados detalhados foram salvos na pasta '$(RESULTS_DIR)/'."
	@echo "Use 'grep \"Tempo de execução\" $(RESULTS_DIR)/*.log' para ver os resumos."
	@echo "Ou abra os arquivos de log individuais para mais detalhes."
