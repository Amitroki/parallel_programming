#include <mpi.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <algorithm>

using namespace std;

template <typename T>
vector<T> generate_random_matrix(size_t rows, size_t cols, T min_val, T max_val) noexcept {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<T> dist(min_val, max_val);

    vector<T> matrix(rows * cols);
    for (auto& elem : matrix) {
        elem = dist(gen);
    }
    return matrix;
}

template <typename T>
void mpi_matrix_multiplication(const vector<T>& A, const vector<T>& B, vector<T>& C,
    int rows1, int cols1, int cols2) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_proc = rows1 / size;
    int remainder = rows1 % size;
    int start_row = rank * rows_per_proc + min(rank, remainder);
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);

    vector<T> local_result(local_rows * cols2, 0);

    for (int i = 0; i < local_rows; ++i) {
        for (int j = 0; j < cols2; ++j) {
            T sum = 0;
            for (int k = 0; k < cols1; ++k) {
                sum += A[(start_row + i) * cols1 + k] * B[k * cols2 + j];
            }
            local_result[i * cols2 + j] = sum;
        }
    }

    vector<int> recv_counts(size), displs(size);
    for (int i = 0; i < size; ++i) {
        int rows = rows_per_proc + (i < remainder ? 1 : 0);
        recv_counts[i] = rows * cols2;
        displs[i] = (i == 0 ? 0 : displs[i - 1] + recv_counts[i - 1]);
    }

    MPI_Gatherv(local_result.data(), local_rows * cols2, MPI_DOUBLE,
        C.data(), recv_counts.data(), displs.data(), MPI_DOUBLE,
        0, MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    const double MIN_VAL = -100000.0;
    const double MAX_VAL = 100000.0;
    const int NUMBER_OF_EXPERIMENTS = 10;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int size = 100; size <= 1000; size += 100) {
        int rows1 = size, cols1 = size;
        int rows2 = size, cols2 = size;

        double total_time = 0.0;

        for (int attempt = 0; attempt < NUMBER_OF_EXPERIMENTS; ++attempt) {
            vector<double> A, B;

            if (rank == 0) {
                A = generate_random_matrix<double>(rows1, cols1, MIN_VAL, MAX_VAL);
                B = generate_random_matrix<double>(rows2, cols2, MIN_VAL, MAX_VAL);
            }

            A.resize(rows1 * cols1);
            B.resize(rows2 * cols2);

            MPI_Bcast(A.data(), A.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Bcast(B.data(), B.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

            vector<double> C(rows1 * cols2);

            MPI_Barrier(MPI_COMM_WORLD);
            auto start = chrono::high_resolution_clock::now();

            mpi_matrix_multiplication(A, B, C, rows1, cols1, cols2);

            MPI_Barrier(MPI_COMM_WORLD);
            auto end = chrono::high_resolution_clock::now();

            if (rank == 0) {
                double duration = chrono::duration<double>(end - start).count();
                total_time += duration;
            }
        }

        if (rank == 0) {
            double avg_time = total_time / NUMBER_OF_EXPERIMENTS;
            cout << "Size: " << size << "x" << size
                << " | Average time over " << NUMBER_OF_EXPERIMENTS
                << " runs: " << fixed << setprecision(6) << avg_time << " sec" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
