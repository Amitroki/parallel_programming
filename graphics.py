import matplotlib.pyplot as plt

# Список файлов и меток для легенды
files = ['1.txt', '2.txt', '4.txt', '8.txt']
labels = ['Набор 1', 'Набор 2', 'Набор 3', 'Набор 4']

# Создаем график
plt.figure(figsize=(10, 6))

for file, label in zip(files, labels):
    x, y = [], []
    with open(file, 'r') as f:
        for line in f:
            if line.strip():  # Пропуск пустых строк
                parts = line.strip().split()
                if len(parts) >= 2:
                    x_val, y_val = map(float, parts[:2])
                    x.append(x_val)
                    y.append(y_val)
    plt.plot(x, y, marker='o', label=label)

plt.xlabel('X')
plt.ylabel('Y')
plt.title('График из нескольких .txt файлов')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()