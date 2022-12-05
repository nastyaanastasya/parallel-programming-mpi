#include <iostream>
#include <mpi.h>
#include <random>
#include <chrono>

int const SRC = 0;
int const DEST = 1;
int const TAG = 8;

int const N = 30000;

void ping_pong(int rank) {
    static int* mess = new int[N];

    if (rank == 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 100);

        for (int i = 0; i < N; i++) {
            mess[i] = dist(gen);
        }

        auto start = std::chrono::high_resolution_clock::now();

        MPI_Send(mess, N, MPI_INT, DEST, TAG, MPI_COMM_WORLD);
        MPI_Recv(mess, N, MPI_INT, DEST, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        auto end = std::chrono::high_resolution_clock::now();

        float time_took = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
        printf("Time consistent %f \n", time_took);
    }
    else {
        int* received = new int[N];

        MPI_Recv(received, N, MPI_INT, SRC, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(received, N, MPI_INT, SRC, TAG, MPI_COMM_WORLD);
    }
}

void ping_ping(int rank) {
    static int* mess = new int[N];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 100);

    for (int i = 0; i < N; i++) {
        mess[i] = dist(gen);
    }

    std::chrono::high_resolution_clock::time_point start;

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        start = std::chrono::high_resolution_clock::now();

        MPI_Send(mess, N, MPI_INT, DEST, TAG, MPI_COMM_WORLD);
    }
    else {
        start = std::chrono::high_resolution_clock::now();

        MPI_Send(mess, N, MPI_INT, SRC, TAG, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        static int* received = new int[N];

        MPI_Recv(received, N, MPI_INT, DEST, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    else {
        static int* mess2 = new int[N];

        MPI_Recv(mess2, N, MPI_INT, SRC, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    auto end = std::chrono::high_resolution_clock::now();

    float time_took = std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count();
    printf("Time consistent %f \n", time_took);
}

int main(int argc, char** argv) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

//    ping_pong(rank);
    ping_ping(rank);

    MPI_Finalize();
}
