import subprocess
import numpy as np
import matplotlib.pyplot as plt

sizes = [10, 50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
interval_double = "-100000.001 100000.001"
interval_int = "1000000 1000000"
paths = [
    "C:/Users/Alex/Desktop/labs/parallel_programming/multiplier/input_data/start_matrix1.txt",
    "C:/Users/Alex/Desktop/labs/parallel_programming/multiplier/input_data/start_matrix2.txt",
    "C:/Users/Alex/Desktop/labs/parallel_programming/multiplier/output_data/end_matrix.txt"
]

executable_path = "C:/Users/Alex/Desktop/labs/parallel_programming/multiplier/out/build/x64-release/multiplier.exe"

graphic_path = "C:/Users/Alex/Desktop/labs/parallel_programming/multiplier/output_data/result.png"

def load_matrix(filepath):
    with open(filepath, "r") as f:
        return np.loadtxt(f)
    
NUMBER_OF_EXPERIMENTS = 10

x = []
y = []

x_avg = []
y_avg = []

for size in sizes:
    time = 0.0
    for _ in range(NUMBER_OF_EXPERIMENTS):
        input_data = f"{size}\n{size}\n{size}\n{size}\n{interval_double}\n"
        process = subprocess.Popen([executable_path, *paths],
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                text=True,
                                bufsize=1
                                )

        stdout_data, stderr_data = process.communicate(input=input_data)

        if stderr_data:
            print(f"C++ ERROR: {stderr_data}")
    
        process.stdin.close()
        process.stdout.close()
        process.stderr.close()
        process.wait()

        A = load_matrix(paths[0])
        B = load_matrix(paths[1])
        C_cpp = load_matrix(paths[2])

        C_numpy = np.dot(A, B)

        if np.allclose(C_numpy, C_cpp):
            print(f"✅ Matrices IS equal for {size} * {size}!")
        else:
            print(f"❌ Matrices is NOT equal for {size} * {size}!")

        lines = stdout_data.strip().split("\n")
        last_line = lines[-1]
        time_value = last_line.split()[-1]

        time += float(time_value)
        x.append(size)
        y.append(float(time_value))
        
    time /= NUMBER_OF_EXPERIMENTS
    print(f"Average for {size} * {size} time of the execution: ", {time})
    x_avg.append(size)
    y_avg.append(time)

plt.scatter(x, y, marker='o', color='b', s = 40)
plt.scatter(x_avg, y_avg, marker='x', color='r', s = 80)

plt.title('Graph of the dependence of the counting speed on the size of the matrices')
plt.xlabel('Matrix sizes, size * size')
plt.ylabel('Time of execution, sec')
plt.grid(True)  
plt.legend()

plt.savefig(graphic_path)
plt.show()