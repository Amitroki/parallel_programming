import subprocess
import numpy as np
import matplotlib.pyplot as plt

sizes = list(range(100, 1100, 100))
value_interval = (-1000000, 1000000)
NUMBER_OF_EXPERIMENTS = 10
process_counts = [1, 2, 4, 8]

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

results = {np_count: {'x': [], 'y': []} for np_count in process_counts}

for np_count in process_counts:
    print(f"\nCompleting {np_count} processes...\n\n")

    for size in sizes:
        total_time = 0.0

        for exp_num in range(NUMBER_OF_EXPERIMENTS):
            input_data = f"{size}\n{size}\n{size}\n{size}\n{value_interval[0]} {value_interval[1]}\n"

            cmd = [
                "mpiexec", "-n", str(np_count),
                executable_path,
                *paths
            ]

            process = subprocess.run(cmd, input=input_data.encode(), capture_output=True)

            if process.returncode != 0:
                print(f"❌ Error: \n{process.stderr.decode()}")
                continue

            stdout = process.stdout.decode()
            lines = stdout.strip().split("\n")
            try:
                exec_time = float(lines[-1].split()[-1])
            except (IndexError, ValueError):
                print(f"❌ Error:\n{lines[-1]}")
                continue

            try:
                A = load_matrix(paths[0])
                B = load_matrix(paths[1])
                C_cpp = load_matrix(paths[2])
                C_np = np.dot(A, B)

                if np.allclose(C_cpp, C_np, atol=1e-3):
                    print(f"✅ Matrices IS equal for {size} * {size}!")
                else:
                    print(f"❌ Matrices is NOT equal for {size} * {size}!")
            except Exception as e:
                print(f"⚠️ Error: {e}")
                continue

            total_time += exec_time

        avg_time = total_time / NUMBER_OF_EXPERIMENTS
        results[np_count]['x'].append(size)
        results[np_count]['y'].append(avg_time)
        print(f"Average for {size} * {size} time of the execution: ", {avg_time})

    print(f"\n✅ Completed for {np_count} processes!\n\n")

for np_count, data in results.items():
    plt.plot(data['x'], data['y'], marker='o', label=f"{np_count} process[es]")

plt.title('Graph of the dependence of the counting speed on the size of the matrices')
plt.xlabel('Matrix sizes, size * size')
plt.ylabel('Time of execution, sec')
plt.grid(True)
plt.legend()
plt.savefig(graphic_path)
plt.show()
