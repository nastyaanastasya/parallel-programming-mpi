#include <iostream>
#include <mpi.h>
#include <random>

int const SRC = 0;
int const TAG = 1;

int const N = 12;

void print(int* a, int start, int length) {
    for (int i = start; i < length; i++) {
        printf("%d ", a[i]);
    }
}

int main(int argc, char** argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int step = N / size + 1;
    int master_cnt = N % step;

    int* a = new int[N];
    int* v;

    if (rank == 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 100);

        for (int i = 0; i < N; i++) {
            a[i] = dist(gen);
            printf("%d ", a[i]);
        }
        printf("\n");

        int start = master_cnt;
        for (int i = 1; i < size; i++) {
            MPI_Send(a + start, step, MPI_INT, i, TAG, MPI_COMM_WORLD);
            start += step;
        }

        printf("Process %d: ", rank);
        print(a, 0, master_cnt);
        printf("\n");
    }
    else {
        v = new int[step];
        MPI_Recv(v, step, MPI_INT, SRC, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Process %d: ", rank);
        if (rank < N / step + 1) {
            print(v, 0, step);
        }
        printf("\n");
    }

    MPI_Finalize();
}
