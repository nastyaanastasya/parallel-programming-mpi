#include <iostream>
#include <mpi.h>
#include <unordered_map>
#include <vector>

using namespace std;

void task_1(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    printf("Hello, World!");
    MPI_Finalize();
}

void task_2(int argc, char *argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Hello from process number %d out of %d \n", rank, size);
    MPI_Finalize();
}

void task_3(int argc, char *argv[]) {

    const int ARRAY_SIZE = 10;
    const int LIMIT = 100;
    const int TAG = 3;

    int rank, a[ARRAY_SIZE];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {

        printf("On rank 0: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] = rand() % LIMIT;
            printf("%d ", a[i]);
        }
        MPI_Send(a, ARRAY_SIZE, MPI_INT, 1, TAG, MPI_COMM_WORLD);
    } else {
        MPI_Recv(a, ARRAY_SIZE, MPI_INT, 0, TAG, MPI_COMM_WORLD, &status);

        printf("On rank 1: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", a[i]);
        }
    }
    MPI_Finalize();
}

void task_4(int argc, char *argv[]) {

    const int ARRAY_SIZE = 20;
    const int LIMIT = 100;
    const int TAG = 4;

    int rank, size, count, a[ARRAY_SIZE];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 1) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            a[i] = rand() % LIMIT;
        }

        MPI_Send(&a, ARRAY_SIZE, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        MPI_Send(&a, ARRAY_SIZE, MPI_INT, 2, TAG, MPI_COMM_WORLD);
        MPI_Send(&a, ARRAY_SIZE, MPI_INT, 3, TAG, MPI_COMM_WORLD);
    } else {
        MPI_Probe(1, TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);
        MPI_Recv(&a, count, MPI_INT, 1, TAG, MPI_COMM_WORLD, &status);

        printf("Process %d out of %d: ", rank, size);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", a[i]);
        }
    }
    MPI_Finalize();
}

void task_5(int argc, char *argv[]) {

    const int ARRAY_SIZE = 15;
    const int LIMIT = 100;
    const int SOURCE = 0;

    const int TAG_SRC = 0;
    const int TAG_DEST = 1;

    const int A = 7;
    const int B = 9;

    int rank, size;
    unordered_map<int, vector<int>> map;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int BLOCK_SIZE = ARRAY_SIZE / (size - 1) + (ARRAY_SIZE % (size - 1) != 0);

    if (rank == SOURCE) {
        int x[ARRAY_SIZE], y[ARRAY_SIZE], z[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            x[i] = rand() % LIMIT;
            y[i] = rand() % LIMIT;
        }

        printf("Process %d:\nVector x: ", rank);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", x[i]);
        }
        printf("\nVector y: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", y[i]);
        }
        printf("\n");

        int dest = 1;
        for (int i = 0; i < ARRAY_SIZE; i += BLOCK_SIZE, dest++) {
            int length = min(BLOCK_SIZE, ARRAY_SIZE - i);
            MPI_Send(&x[i], length, MPI_INT, dest, TAG_SRC, MPI_COMM_WORLD);
            MPI_Send(&y[i], length, MPI_INT, dest, TAG_SRC, MPI_COMM_WORLD);
            map[dest] = {i, i + length};
        }

        int waiters = size - 1;
        while (waiters > 0) {
            int count;
            MPI_Status status;

            MPI_Probe(MPI_ANY_SOURCE, TAG_DEST, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);

            int received_data[count];
            MPI_Recv(&received_data, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

            vector<int> data = map.at(status.MPI_SOURCE);
            int i = 0;
            for (int k = data[0]; k < data[1]; k++, i++) {
                z[k] = received_data[i];
            }
            waiters--;
        }

        printf("Result z: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", z[i]);
        }
        printf("\n\n");
    } else {
        int count;
        MPI_Status status;

        MPI_Probe(SOURCE, TAG_SRC, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        int x[count], y[count], z[count];

        MPI_Recv(&x, count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&y, count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

        printf("Process %d:\nx: ", rank);
        for (int i = 0; i < count; i++) {
            printf("%d ", x[i]);
        }
        printf("\ny: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", y[i]);
        }

        for (int i = 0; i < count; i++) {
            z[i] = A * x[i] + B * y[i];
        }
        printf("\nz: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", z[i]);
        }
        printf("\n\n");

        MPI_Send(&z, count, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}


int main(int argc, char *argv[]) {
    srand(time(0));

    task_1(argc, argv);
    task_2(argc, argv);
    task_3(argc, argv);
    task_4(argc, argv);
    task_5(argc, argv);

    return 0;
}
