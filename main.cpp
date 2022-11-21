#include <cmath>
#include <ctime>
#include <iostream>
#include <mpi.h>
#include <unordered_map>
#include <vector>

using namespace std;

void task1_1(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    printf("Hello, World!");
    MPI_Finalize();
}

void task1_2(int argc, char *argv[]) {
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Hello from process number %d out of %d \n", rank, size);
    MPI_Finalize();
}

void task1_3(int argc, char *argv[]) {

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

void task1_4(int argc, char *argv[]) {

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

void task1_5(int argc, char *argv[]) {

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

void task2_1(int argc, char *argv[]) {

    const int ARRAY_SIZE = 20;
    const int LIMIT = 20;
    const int SOURCE = 0;

    const int TAG_SRC = 0;
    const int TAG_DEST = 1;

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
            z[i] = x[i] * y[i];
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

void task2_2(int argc, char *argv[]) {

    const int ARRAY_SIZE = 15;
    const int LIMIT = 100;
    const int SOURCE = 0;

    const int TAG_SRC = 0;
    const int TAG_DEST = 1;

    int rank, size;
    unordered_map<int, vector<int>> map;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int BLOCK_SIZE = ARRAY_SIZE / (size - 1) + (ARRAY_SIZE % (size - 1) != 0);

    if (rank == SOURCE) {
        int x[ARRAY_SIZE], y[ARRAY_SIZE];
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

        for (int i = 0, dest = 1; i < ARRAY_SIZE; i += BLOCK_SIZE, dest++) {
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

            int recv_x[count];
            int recv_y[count];
            MPI_Recv(&recv_x, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(&recv_y, count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

            vector<int> data = map.at(status.MPI_SOURCE);
            for (int k = data[0], i = 0; k < data[1]; k++, i++) {
                x[k] = recv_x[i];
                y[k] = recv_y[i];
            }
            waiters--;
        }

        printf("Result x: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", x[i]);
        }
        printf("\nResult y: ");
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%d ", y[i]);
        }
        printf("\n\n");
    } else {
        int count;
        MPI_Status status;

        MPI_Probe(SOURCE, TAG_SRC, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        int x[count], y[count];

        MPI_Recv(&x, count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&y, count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

        printf("Process %d:\nReceived x: ", rank);
        for (int i = 0; i < count; i++) {
            printf("%d ", x[i]);
        }
        printf("\nReceived y: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", y[i]);
        }

        for (int i = 0; i < count; i++) {
            int tmp = y[i];
            y[i] = x[i];
            x[i] = tmp;
        }
        printf("\nResult x: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", x[i]);
        }
        printf("\nResult y: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", y[i]);
        }
        printf("\n\n");

        MPI_Send(&x, count, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
        MPI_Send(&y, count, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

void task2_3(int argc, char *argv[]) {

    const int LIMIT = 10;
    const int MATRIX_SIZE = 6;
    const int SOURCE = 0;

    const int TAG_SRC = 0;
    const int TAG_DEST = 1;

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int BLOCK_SIZE = MATRIX_SIZE * MATRIX_SIZE / (size - 1) + (MATRIX_SIZE * MATRIX_SIZE % (size - 1) != 0);
    const int LAST_BLOCK = MATRIX_SIZE * MATRIX_SIZE - BLOCK_SIZE * (size - 2);

    if (rank == SOURCE) {
        int a[MATRIX_SIZE * MATRIX_SIZE], b[MATRIX_SIZE * MATRIX_SIZE];
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                a[i * MATRIX_SIZE + j] = rand() % LIMIT;
                b[i * MATRIX_SIZE + j] = rand() % LIMIT;
            }
        }

        printf("\nMatrix A:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", a[i * MATRIX_SIZE + j]);
            }
            printf("\n");
        }
        printf("\nMatrix B:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", b[i * MATRIX_SIZE + j]);
            }
            printf("\n");
        }
        printf("\n");

        for (int dest = 1; dest < size; dest++) {
            int num;
            if (size - dest != 1) {
                num = BLOCK_SIZE;
            } else {
                num = LAST_BLOCK;
            }
            MPI_Send(&a[(dest - 1) * BLOCK_SIZE], num, MPI_INT, dest, TAG_SRC, MPI_COMM_WORLD);
            MPI_Send(&b[(dest - 1) * BLOCK_SIZE], num, MPI_INT, dest, TAG_SRC, MPI_COMM_WORLD);
        }

        int c[MATRIX_SIZE * MATRIX_SIZE];
        for (int dest = 1; dest < size; dest++) {

            int count;
            if (size - dest != 1) {
                count = BLOCK_SIZE;
            } else {
                count = LAST_BLOCK;
            }
            MPI_Recv(&c[(dest - 1) * BLOCK_SIZE], count, MPI_INT, dest, TAG_DEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Result matrix C:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", c[i * MATRIX_SIZE + j]);
            }
            printf("\n");
        }
        printf("\n");
    } else {
        int count;
        MPI_Status status;

        MPI_Probe(SOURCE, TAG_SRC, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &count);

        int a[count], b[count], c[count];

        MPI_Recv(&a[0], count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&b[0], count, MPI_INT, SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

        for (int i = 0; i < count; i++) {
            c[i] = a[i] * b[i];
        }

        MPI_Send(&c[0], count, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

void task2_4(int argc, char *argv[]) {

    const int LIMIT = 10;
    const int MATRIX_SIZE = 6;
    const int SOURCE = 0;

    const int TAG_SRC_A = 10;
    const int TAG_SRC_B = 20;
    const int TAG_DEST = 1;

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int BLOCK_SIZE = size < MATRIX_SIZE ? MATRIX_SIZE / (size - 1) + (MATRIX_SIZE % (size - 1) != 0) :
                           MATRIX_SIZE / MATRIX_SIZE + (MATRIX_SIZE % (size - 1) != 0);

    if (rank == SOURCE) {
        int a[MATRIX_SIZE][MATRIX_SIZE],
                b[MATRIX_SIZE][MATRIX_SIZE],
                c[MATRIX_SIZE][MATRIX_SIZE];

        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                a[i][j] = rand() % LIMIT;
                b[i][j] = rand() % LIMIT;
            }
        }

        printf("\nMatrix A:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", a[i][j]);
            }
            printf("\n");
        }
        printf("\nMatrix B:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", b[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        for (int i = 0, dest = 1; i < MATRIX_SIZE; i += BLOCK_SIZE, dest++) {
            int count = BLOCK_SIZE < MATRIX_SIZE - i ? BLOCK_SIZE : MATRIX_SIZE - i;
            MPI_Send(&(a[i][0]), count * MATRIX_SIZE, MPI_INT, dest, TAG_SRC_A, MPI_COMM_WORLD);
            MPI_Send(&(b[0][0]), MATRIX_SIZE * MATRIX_SIZE, MPI_INT, dest, TAG_SRC_B, MPI_COMM_WORLD);
        }

        for (int i = 0, dest = 1; i < MATRIX_SIZE; i += BLOCK_SIZE, dest++) {
            int count = BLOCK_SIZE < MATRIX_SIZE - i ? BLOCK_SIZE : MATRIX_SIZE - i;
            MPI_Recv(&(c[i][0]), count * MATRIX_SIZE, MPI_INT, dest, TAG_DEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Result matrix C:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", c[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    } else {
        int cnt_a;
        int cnt_b;

        MPI_Status status;

        MPI_Probe(SOURCE, TAG_SRC_A, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &cnt_a);

        MPI_Probe(SOURCE, TAG_SRC_B, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &cnt_b);

        int line_num = (int) sqrt(cnt_b);
        int count = cnt_a / line_num;

        int a[count][MATRIX_SIZE],
                b[MATRIX_SIZE][MATRIX_SIZE],
                c[count][MATRIX_SIZE];

        MPI_Recv(&(a[0][0]), cnt_a, MPI_INT, SOURCE, TAG_SRC_A, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(b[0][0]), cnt_b, MPI_INT, SOURCE, TAG_SRC_B, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < count; i++) {
            for (int j = 0; j < line_num; j++) {
                c[i][j] = 0;
                for (int k = 0; k < line_num; k++) {
                    c[i][j] += a[i][k] * b[k][j];
                }
            }
        }
        MPI_Send(&(c[0][0]), cnt_a, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

void task2_5(int argc, char *argv[]) {

    const int LIMIT = 10;
    const int MATRIX_SIZE = 6;
    const int SOURCE = 0;

    const int TAG_SRC_A = 10;
    const int TAG_SRC_C = 20;
    const int TAG_DEST = 1;

    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int BLOCK_SIZE = size < MATRIX_SIZE ? MATRIX_SIZE / (size - 1) + (MATRIX_SIZE % (size - 1) != 0) :
                           MATRIX_SIZE / MATRIX_SIZE + (MATRIX_SIZE % (size - 1) != 0);

    if (rank == SOURCE) {
        int a[MATRIX_SIZE][MATRIX_SIZE],
                b[MATRIX_SIZE][MATRIX_SIZE],
                c[MATRIX_SIZE][MATRIX_SIZE];

        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                a[i][j] = rand() % LIMIT;
                c[i][j] = a[i][j];
            }
        }

        printf("\nMatrix A:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", a[i][j]);
            }
            printf("\n");
        }

        for (int i = 0, dest = 1; i < MATRIX_SIZE; i += BLOCK_SIZE, dest++) {
            int count = BLOCK_SIZE < MATRIX_SIZE - i ? BLOCK_SIZE : MATRIX_SIZE - i;
            MPI_Send(&(a[i][0]), count * MATRIX_SIZE, MPI_INT, dest, TAG_SRC_A, MPI_COMM_WORLD);
            MPI_Send(&(c[0][0]), MATRIX_SIZE * MATRIX_SIZE, MPI_INT, dest, TAG_SRC_C, MPI_COMM_WORLD);
        }

        for (int i = 0, dest = 1; i < MATRIX_SIZE; i += BLOCK_SIZE, dest++) {
            int count = BLOCK_SIZE < MATRIX_SIZE - i ? BLOCK_SIZE : MATRIX_SIZE - i;
            MPI_Recv(&(b[i][0]), count * MATRIX_SIZE, MPI_INT, dest, TAG_DEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("\nResult matrix B:\n");
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                printf("%d ", b[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    } else {
        int cnt_a;
        int cnt_c;

        MPI_Status status;

        MPI_Probe(SOURCE, TAG_SRC_A, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &cnt_a);

        MPI_Probe(SOURCE, TAG_SRC_C, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_INT, &cnt_c);

        int col_num = (int) sqrt(cnt_c);
        int count = cnt_a / col_num;

        int a[count][MATRIX_SIZE],
                c[MATRIX_SIZE][MATRIX_SIZE];

        MPI_Recv(&(a[0][0]), cnt_a, MPI_INT, SOURCE, TAG_SRC_A, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&(c[0][0]), cnt_c, MPI_INT, SOURCE, TAG_SRC_C, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < count; i++) {
            for (int j = 0; j < col_num; j++) {
                a[i][j] = c[j][i + count * (rank - 1)];
            }
        }
        MPI_Send(&(a[0][0]), cnt_a, MPI_INT, SOURCE, TAG_DEST, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

void task3_1(int argc, char *argv[]) {

    int rank, size;

    const int LIMIT = 10;
    const int N = 20;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int BLOCK_SIZE = N % size == 0 ? N / size : N / size + 1;

    int x[N], y[BLOCK_SIZE];

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            x[i] = rand() % LIMIT;
            printf("%d ", x[i]);
        }
        printf("\n");
    }

    MPI_Scatter(&x[rank * BLOCK_SIZE], BLOCK_SIZE, MPI_INT, &y[0], BLOCK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    for (int i = 0; i < BLOCK_SIZE && rank * BLOCK_SIZE + i < N; i++) {
        sum += abs(y[i]);
    }

    int result;
    MPI_Reduce(&sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result sum: %d\n", result);
    }
    MPI_Finalize();
}

void task3_2(int argc, char *argv[]) {

    int rank, size;

    const int LIMIT = 10;
    const int N = 20;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int x[N], y[N], x_local[N], y_local[N];

    const int BLOCK_SIZE = N % size == 0 ? N / size : N / size + 1;

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            x[i] = rand() % LIMIT;
            y[i] = rand() % LIMIT;
        }

        printf("Vector x: ");
        for (int i = 0; i < N; i++) {
            printf("%d ", x[i]);
        }
        printf("\n");

        printf("Vector y: ");
        for (int i = 0; i < N; i++) {
            printf("%d ", y[i]);
        }
        printf("\n");
    }

    MPI_Scatter(&x[rank * BLOCK_SIZE], BLOCK_SIZE, MPI_INT, &x_local[0], BLOCK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&y[rank * BLOCK_SIZE], BLOCK_SIZE, MPI_INT, &y_local[0], BLOCK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    int sum = 0;
    for (int i = 0; i < BLOCK_SIZE && rank * BLOCK_SIZE + i < N; i++) {
        sum += x_local[i] * y_local[i];
    }

    int result;
    MPI_Reduce(&sum, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result: %d\n", result);
    }
    MPI_Finalize();
}

void task3_3(int argc, char *argv[]) {

    int rank, size;

    const int LIMIT = 10;
    const int N = 5;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int a[N][N], b[N];

    const int BLOCK_SIZE = N % size == 0 ? N / size : N / size + 1;

    if (rank == 0) {
        printf("Matrix A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                a[i][j] = rand() % LIMIT;
                printf("%d ", a[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("Vector b: ");
        for (int i = 0; i < N; i++) {
            b[i] = rand() % LIMIT;
            printf("%d ", b[i]);
        }
        printf("\n\n");
    }

    int local_a[BLOCK_SIZE][N];

    MPI_Bcast(&b[0], N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(&a[rank * BLOCK_SIZE][0], BLOCK_SIZE * N, MPI_INT, &local_a[0][0], BLOCK_SIZE * N, MPI_INT, 0, MPI_COMM_WORLD);

    int local_res[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE && rank * BLOCK_SIZE + i < N; i++) {
        local_res[i] = 0;
        for (int j = 0; j < N; j++) {
            local_res[i] += local_a[i][j] * b[j];
        }
    }

    int result[N];
    MPI_Gather(&local_res[0], BLOCK_SIZE, MPI_INT, &result[rank * BLOCK_SIZE], BLOCK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result: ");
        for (int i = 0; i < N; i++) {
            printf("%d ", result[i]);
        }
        printf("\n");
    }
    MPI_Finalize();
}

void task3_4(int argc, char *argv[]) {

    int rank, size;

    const int LIMIT = 10;
    const int N = 5, M = 4;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int result;
    int a[N][M];

    const int BLOCK_SIZE = N % size == 0 ? N / size : N / size + 1;

    if (rank == 0) {

        printf("Matrix a:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                a[i][j] = rand() % LIMIT;
                printf("%d ", a[i][j]);
            }
            printf("\n");
        }
    }

    int a_local[BLOCK_SIZE][M];
    MPI_Scatter(&a[rank * BLOCK_SIZE][0], BLOCK_SIZE * M, MPI_INT, &a_local[0][0], BLOCK_SIZE * M, MPI_INT, 0,MPI_COMM_WORLD);

    int sum;
    int max_value = INT_MIN;
    for (int i = 0; i < BLOCK_SIZE && rank * BLOCK_SIZE + i < N; i++) {
        sum = 0;
        for (int j = 0; j < M; j++) {
            sum += a_local[i][j];
        }
        max_value = sum > max_value ? sum : max_value;
    }

    MPI_Reduce(&max_value, &result, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result: %d\n", result);
    }
    MPI_Finalize();
}

void task3_5(int argc, char *argv[]) {

    int rank, size;

    const int LIMIT = 10;
    const int N = 5;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int a[N][N], b[N][N], c[N][N];

    const int BLOCK_SIZE = N % size == 0 ? N / size : N / size + 1;

    if (rank == 0) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                a[i][j] = rand() % LIMIT;
                b[i][j] = rand() % LIMIT;
            }
        }

        printf("Matrix A:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", a[i][j]);
            }
            printf("\n");
        }
        printf("\n");

        printf("Matrix B:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", b[i][j]);
            }
            printf("\n");
        }
        printf("\n");
    }

    int local_a[BLOCK_SIZE][N],
            local_b[BLOCK_SIZE][N],
            local_c[BLOCK_SIZE][N];

    MPI_Scatter(&a[rank * BLOCK_SIZE][0], BLOCK_SIZE * N, MPI_INT, &local_a[0][0], BLOCK_SIZE * N, MPI_INT, 0,
                MPI_COMM_WORLD);
    MPI_Scatter(&b[rank * BLOCK_SIZE][0], BLOCK_SIZE * N, MPI_INT, &local_b[0][0], BLOCK_SIZE * N, MPI_INT, 0,
                MPI_COMM_WORLD);

    for (int i = 0; i < BLOCK_SIZE && rank * BLOCK_SIZE + i < N; i++) {
        for (int j = 0; j < N; j++) {
            local_c[i][j] = local_a[i][j] * local_b[i][j];
        }
    }
    MPI_Gather(&local_c[0][0], BLOCK_SIZE * N, MPI_INT, &c[rank * BLOCK_SIZE][0], BLOCK_SIZE * N, MPI_INT, 0,
               MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Result matrix C:\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", c[i][j]);
            }
            printf("\n");
        }
    }
    MPI_Finalize();
}

int main(int argc, char *argv[]) {
    srand(time(0));

    return 0;
}
