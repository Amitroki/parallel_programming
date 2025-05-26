#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
#include <stdexcept>
#include <iomanip>

using namespace std;

template <typename T>
auto generate_random_matrix(size_t rows, size_t cols, T min_val, T max_val) noexcept {
    random_device rd;
    mt19937 gen(rd());

    if constexpr (is_floating_point_v<T>) {
        uniform_real_distribution<T> dist(min_val, max_val);
        vector<T> matrix(rows * cols);
        for (auto& elem : matrix) {
            elem = dist(gen);
        }
        return matrix;
    }
    else {
        uniform_int_distribution<T> dist(min_val, max_val);
        vector<T> matrix(rows * cols);
        for (auto& elem : matrix) {
            elem = dist(gen);
        }
        return matrix;
    }
}

template <typename T>
void write_matrix(string path, const vector<T>& matrix, int rows, int cols) {
    ofstream file(path);
    if (!file.is_open()) {
        throw runtime_error("File opening error.");
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file << fixed << setprecision(7) << matrix[i * cols + j] << ' ';
        }
        file << '\n';
    }
}

template <typename T>
void mpi_matrix_multiplication(const vector<T>& A, const vector<T>& B, vector<T>& C, int rows1, int cols1, int cols2) {
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

template <typename T>
void matrix_processing(const string& first_matrix_path, const string& second_matrix_path, const string& result_path,
    int rows1, int cols1, int rows2, int cols2, T min, T max) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    vector<T> A, B;

    if (rank == 0) {
        A = generate_random_matrix<T>(rows1, cols1, min, max);
        B = generate_random_matrix<T>(rows2, cols2, min, max);
        write_matrix(first_matrix_path, A, rows1, cols1);
        write_matrix(second_matrix_path, B, rows2, cols2);
    }

    A.resize(rows1 * cols1);
    B.resize(rows2 * cols2);

    MPI_Bcast(A.data(), A.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B.data(), B.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

    vector<T> C(rows1 * cols2);

    MPI_Barrier(MPI_COMM_WORLD);
    auto start = chrono::high_resolution_clock::now();

    mpi_matrix_multiplication<T>(A, B, C, rows1, cols1, cols2);

    MPI_Barrier(MPI_COMM_WORLD);
    auto end = chrono::high_resolution_clock::now();

    if (rank == 0) {
        write_matrix(result_path, C, rows1, cols2);
        double duration = chrono::duration<double>(end - start).count();
        cout << "Time: " << duration << "\n";
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 4) {
        if (rank == 0) {
            cerr << "Usage: <exec> <path A> <path B> <path C>\n";
        }
        MPI_Finalize();
        return 1;
    }

    string path_fm = argv[1];
    string path_sm = argv[2];
    string result_m = argv[3];

    int rows1, cols1, rows2, cols2;
    double min_val, max_val;

    if (rank == 0) {
        cin >> rows1 >> cols1 >> rows2 >> cols2 >> min_val >> max_val;

        if (cols1 != rows2) {
            cerr << "Bad matrix dimensions.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&rows1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols1, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols2, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&min_val, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_val, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    matrix_processing<double>(path_fm, path_sm, result_m, rows1, cols1, rows2, cols2, min_val, max_val);

    MPI_Finalize();
    return 0;
}
