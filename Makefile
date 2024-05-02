all: test simulation_1 simulation_2_from_data

test:
	g++ -O3 test.cpp -o test.out

test-par:
	g++ -fopenmp -O3 test.cpp -o test.out

simulation_1:
	h5c++ -O3 simulation_1.cpp -o simulation_1.out

simulation_2_from_data:
	h5c++ -O3 simulation_2_from_data.cpp -o simulation_2_from_data.out

simulation_3_convergence_time:
	h5c++ -O3 simulation_3_convergence_time.cpp -o simulation_3_convergence_time.out