/******************************************************************************
* FILE: Assignment II - Matrix Calculation Using MPI 
* PATTERN : block, * 
* DESCRIPTION :
* A[i][j] = B[i][j] * C[i][j];
* AUTHOR : Nattapon Jutamas 5710110137
* *****************************************************************************/
#include <stdio.h>
#include <mpi.h>

#define N 30			// number of rows and column in matrix
#define BLOCK_SIZE 4 	// size of the block
#define ROOT 0			// rank 0

void sub_block(int a[N][N], int b[N][N], int c[N][N], int m[N][N], int br[N][N], int bm[N][N], int MyRank, int rstart) {
	for (int row = rstart; (row < rstart + BLOCK_SIZE) && (row < N); row++) {
		for (int col = 0; col < N; col++) {
			if (MyRank == ROOT) {
				a[row][col] = b[row][col] * c[row][col];
				m[row][col] = MyRank;
			}
			br[row][col] = b[row][col] * c[row][col];
			bm[row][col] = MyRank;
			//printf("MyRank:%d process at index(%d,%d)\n", MyRank, row, col);
		}
	}
}

int main(int argc, char *argv[])
{
	int MyRank, Numprocs;		// for storing this process' MyRank, and the number of processes
	int A[N][N];				// result matrix C
	int B[N][N];				// matrix B to be multiplied
	int C[N][N];				// matrix C to be multiplied
	int matrix_mask[N][N];
	int nblocks;
	int remain_block;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
	MPI_Comm_size(MPI_COMM_WORLD, &Numprocs);

	remain_block = N % BLOCK_SIZE;
	if (remain_block) {
		// 0 = all blocks are same size, otherwise last block size = remain_block
		nblocks = (N / BLOCK_SIZE) + 1;
	}
	else {
		nblocks = N / BLOCK_SIZE;
	}

	if (MyRank == ROOT) {
		printf("The equation is A[i][j] = B[i][j] * C[i][j]\n");
		printf("N = %d, Size of the block = %d\n", N, BLOCK_SIZE);
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

	// Parallel Process (block, *)
	int Buffer_Result[N][N];
	int Buffer_Mask[N][N];
	
	for (int row = MyRank; row < nblocks; row += Numprocs) {
		sub_block(A, B, C, matrix_mask, Buffer_Result, Buffer_Mask, MyRank, row * BLOCK_SIZE);
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
			for (int i = source; i < nblocks; i += Numprocs) {
				int rstart = i * BLOCK_SIZE;
				for (int row = rstart; (row < rstart + BLOCK_SIZE) && (row < N); row++) {
					for (int col = 0; col < N; col++) {
						matrix_mask[row][col] = Recv_Mask[row][col];
						A[row][col] = Recv_Result[row][col];
					}
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
		printf("Domain that each thread process(block, *) :\n");
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