import matplotlib.pyplot as plt
import numpy as np
import sys
import csv
from pathlib import Path

def read_csv(output_file):
    data = [[], [], [], []]
    with open(output_file) as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            data[0].append(int(row["time"]))
            data[1].append(float(row["grass"]))
            data[2].append(float(row["prey"]))
            data[3].append(float(row["predator"]))
    return data

def plot(output_files):
    index=1
    plt.figure()
    num_rows=len(output_files)
    for output_file in output_files:
        data = read_csv(output_file)
        plt.subplot(num_rows, 1, index)
        plt.title("Prey Predator model simulation results (" +\
                Path(output_file).stem + ")")
        plt.plot(data[0], data[1], label="Grass")
        plt.plot(data[0], data[2], label="Prey")
        plt.plot(data[0], data[3], label="Predator")
        plt.xlabel("Time Step")
        plt.xticks(np.arange(data[0][0], data[0][-1], step=100))
        max_y = max(max(data[1]),max(data[2]),max(data[3]))
        step_y = max_y / 10
        plt.ylabel("Global Population")
        plt.yticks(np.arange(0, max_y, step=step_y))
        index+=1
        plt.legend()
    plt.show()

if __name__ == "__main__":
    files = [sys.argv[i] for i in range(1, len(sys.argv))]
    plot(files)
