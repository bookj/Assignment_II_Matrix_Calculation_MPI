/******************************************************************************
* FILE: Assignment II - Matrix Calculation Using MPI 
* PATTERN : cyclic, cyclic 
* DESCRIPTION :
* A[i][j] = B[i][j] * C[i][j];
* AUTHOR : Nattapon Jutamas 5710110137
* *****************************************************************************/
#include <stdio.h>
#include <mpi.h>

#define N 30		// number of rows and column in matrix
#define ROOT 0		// rank 0

int main(int argc, char *argv[])
{
	int MyRank, Numprocs;		// for storing this process' MyRank, and the number of processes
	int A[N][N];				// result matrix C
	int B[N][N];				// matrix B to be multiplied
	int C[N][N];				// matrix C to be multiplied
	int matrix_mask[N][N];

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
	MPI_Comm_size(MPI_COMM_WORLD, &Numprocs);

	if (MyRank == ROOT) {
		printf("The equation is A[i][j] = B[i][j] * C[i][j]\n");
		printf("N = %d\n", N);
		printf("Number of Processes = %d\n", Numprocs);
	}	// MyRank == 0 (ROOT)

	// Initialize matrices
	// matrix A
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			A[i][j] = 0;
		}
	}

	// matrix B
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			B[i][j] = (i + j) % 10;
		}
	}

	// matrix C
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			C[i][j] = (i * j) % 10;
		}
	}

	// matrix mask
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			matrix_mask[i][j] = 9;
		}
	}

	if (MyRank == ROOT) {
		printf("******************************************************\n");
		printf("Input Matrix B :\n");
		printf("B[%d][%d]\n", N, N);
		for (int i = 0; i < N; i++) {
			printf("Row:%02d\n", i);
			for (int j = 0; j < N; j++) {
				if (j == 50) {
					printf("\n");
				}
				printf("%2d ", B[i][j]);

			}
			printf("\n");
		}
		printf("******************************************************\n");

		printf("******************************************************\n");
		printf("Input Matrix C :\n");
		printf("C[%d][%d]\n", N, N);
		for (int i = 0; i < N; i++) {
			printf("Row:%02d\n", i);
			for (int j = 0; j < N; j++) {
				if (j == 50) {
					printf("\n");
				}
				printf("%2d ", C[i][j]);

			}
			printf("\n");
		}
		printf("******************************************************\n");
	}	// MyRank == 0 (ROOT)

	// Parallel Process (cyclic, cyclic)
	int Buffer_Result[N][N];
	int Buffer_Mask[N][N];

	for (int row = 0; row < N; row++) {
		int colstart = MyRank - ((row * N) % Numprocs);
		if (colstart < 0) {
			colstart += Numprocs;
		}
		for (int col = colstart; col < N; col += Numprocs) {
			if (MyRank == ROOT) {
				A[row][col] = B[row][col] * C[row][col];
				matrix_mask[row][col] = MyRank;
			}
			Buffer_Result[row][col] = B[row][col] * C[row][col]; 
			Buffer_Mask[row][col] = MyRank;
			//printf("MyRank:%d process at index(%d,%d)\n", MyRank, row, col);
		}
	}

	if (MyRank != ROOT) {
		MPI_Send(Buffer_Result, N * N, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
		MPI_Send(Buffer_Mask, N * N, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
	}
	else {
		for (int source = 1; source < Numprocs; source++) {
			int Recv_Result[N][N];
			int Recv_Mask[N][N];
			MPI_Recv(Recv_Result, N * N, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
			MPI_Recv(Recv_Mask, N * N, MPI_INT, source, 0, MPI_COMM_WORLD, &status);
			for (int row = 0; row < N; row++) {
				int colstart = source - ((row * N) % Numprocs);
				if (colstart < 0) {
					colstart += Numprocs;
				}
				for (int col = colstart; col < N; col += Numprocs) {
					matrix_mask[row][col] = Recv_Mask[row][col];
					A[row][col] = Recv_Result[row][col];
				}
			}
		}
		printf("******************************************************\n");
		printf("Result Matrix A :\n");
		printf("A[%d][%d]\n", N, N);
		for (int i = 0; i < N; i++) {
			printf("Row:%02d\n", i);
			for (int j = 0; j < N; j++) {
				if (j == 50) {
					printf("\n");
				}
				printf("%2d ", A[i][j]);
			}
			printf("\n");
		}
		printf("******************************************************\n");

		printf("******************************************************\n");
		printf("Domain that each thread process(cyclic, cyclic) :\n");
		printf("matrix_mask[%d][%d]\n", N, N);
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				printf("%d ", matrix_mask[i][j]);
			}
			printf("\n");
		}
		printf("******************************************************\n");
	}	// MyRank == 0 (ROOT)

	MPI_Finalize();
	return 0;
}

/*
You're only assigning a in process 0. MPI doesn't share memory, so if you want the a in process 1 
to get the value of 4, you need to call MPI_Send from process 0 and MPI_Recv from process 1.
Ref : https://stackoverflow.com/questions/2059473/mpi-barrier-c

*/