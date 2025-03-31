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
	if (!is_arithmetic_v<T>) {
		throw invalid_argument("Диапазон должен быть числовым.");
	}

	random_device rd;
	mt19937 gen(rd());

	if constexpr (is_floating_point_v<T>) {
		uniform_real_distribution<T> dist(min_val, max_val);
		vector<vector<T>> matrix(rows, vector<T>(cols));
		for (auto& row : matrix) {
			for (auto& elem : row) {
				elem = dist(gen);
			}
		}
		return matrix;
	}
	else if constexpr (is_integral_v<T>) {
		uniform_int_distribution<T> dist(min_val, max_val);
		vector<vector<T>> matrix(rows, vector<T>(cols));
		for (auto& row : matrix) {
			for (auto& elem : row) {
				elem = dist(gen);
			}
		}
		return matrix;
	}
	else {
		throw invalid_argument("Incorrect type of argument");
	}
}

template <typename T>
vector<vector<T>> multiply_two_matrices(vector<vector<T>> matrix1, vector<vector<T>> matrix2) {
	if (matrix1.empty() || matrix2.empty()) {
		throw invalid_argument("There is no matrix");
	}
	int rows1 = matrix1.size();
	int cols1 = matrix1[0].size();
	int rows2 = matrix2.size();
	int cols2 = matrix2[0].size();
	cout << cols1 << "  " << rows2 << endl;
	if (cols1 != rows2) {
		throw invalid_argument("Bad sizes of matrices");
	}
	vector<vector<T>> result(rows1, vector<T>(cols2, 0));
	for (int i = 0; i < rows1; ++i) {
		for (int j = 0; j < cols2; ++j) {
			for (int k = 0; k < cols1; ++k) {
				result[i][j] += matrix1[i][k] * matrix2[k][j];
			}
		}
	}
	return result;
}

template <typename T>
vector<vector<T>> reading_matrix(string path) {
	ifstream file(path);
	vector<vector<T>> matrix;

	if (file.is_open()) {
		string line;
		T num;
		while (getline(file, line) && line != "") {
			vector<T> row;
			stringstream ss(line);
			while (ss >> num) {
				row.push_back(num);
			}
			matrix.push_back(row);
		}
	}

	return matrix;
}

template <typename T>
void write_matrix(string path, vector<vector<T>> matrix) {
	if (matrix.empty()) {
		throw invalid_argument("There is no matrix");
	}

	ofstream file(path);

	if (!file.is_open()) {
		throw runtime_error("Ошибка открытия файла.");
	}

	int rows = matrix.size();
	int cols = matrix[0].size();

	if (file.is_open()) {
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				file << fixed << setprecision(3) << matrix[i][j] << ' ';
			}
			file << '\n';
		}
	}
}

template <typename T>
void matrix_processing(string first_matrix_path, string second_matrix_path, string result_path, int rows1, int cols1, int rows2, int cols2, T min, T max) {
	auto matrix1 = generate_random_matrix<T>(rows1, cols1, min, max);
	write_matrix<T>(first_matrix_path, matrix1);
	auto matrix2 = generate_random_matrix<T>(rows2, cols2, min, max);
	write_matrix<T>(second_matrix_path, matrix2);
	matrix1 = reading_matrix<T>(first_matrix_path);
	matrix2 = reading_matrix<T>(second_matrix_path);
	auto start = chrono::high_resolution_clock::now();
	auto output_matrix = multiply_two_matrices<T>(matrix1, matrix2);
	auto end = chrono::high_resolution_clock::now();
	double duration = chrono::duration<double>(end - start).count();
	write_matrix<T>(result_path, output_matrix);
	cout << "Time: " << duration << '\n' << flush;
	cout.flush();
}

int main(int argc, char* argv[]) {
	// cd cd Desktop/labs/parallel_programming/multiplier/out/build/x64-debug
	// multiplier C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\start_matrix1.txt C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\start_matrix2.txt C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\end_matrix.txt

	// OR

	// C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\out\build\x64-release\multiplier.exe C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\start_matrix1.txt C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\start_matrix2.txt C:\Users\Alex\Desktop\labs\parallel_programming\multiplier\end_matrix.txt
	if (argc < 4) {
		throw "Don't try to trick me, Tony, don't even try";
	}

	cout << "Print the number of rows in first matrix: " << endl << flush;
	int rows1;
	cin >> rows1;
	cout << "Print the number of columns in first matrix: \n" << flush;
	int cols1;
	cin >> cols1;
	cout << "Print the number of rows in second matrix: \n" << flush;
	int rows2;
	cin >> rows2;
	cout << "Print the number of columns in second matrix: \n" << flush;
	int cols2;
	cin >> cols2;

	if (cols1 != rows2) {
		throw invalid_argument("Bad sizes of matrices");
	}

	cout << "\nPrint the range: \n" << flush;
	string lb, ub;
	cin >> lb >> ub;

	bool is_double = lb.find('.') != std::string::npos || ub.find('.') != std::string::npos;
	string path_fm = argv[1];
	string path_sm = argv[2];
	string result_m = argv[3];

	if (is_double) {
		matrix_processing(path_fm, path_sm, result_m, rows1, cols1, rows2, cols2, stod(lb), stod(ub));
	}
	else {
		matrix_processing(path_fm, path_sm, result_m, rows1, cols1, rows2, cols2, stoi(lb), stoi(ub));
	} 

	return 0;
}
