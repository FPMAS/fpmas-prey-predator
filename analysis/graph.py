import matplotlib.pyplot as plt
import numpy as np
import sys
import csv
from pathlib import Path
import re

def read_csv(output_file, mode):
    data = [[], [], []]
    with open(output_file) as csv_file:
        csv_reader = csv.DictReader(csv_file)
        for row in csv_reader:
            data[0].append(int(row["time"]))
            data[1].append(int(row["local_" + mode]))
            data[2].append(int(row["total_" + mode]))
    return data

def plot_graph(output_directory):
    index=1
    plt.figure()
    for mode in ["nodes", "edges"]:
        graph_files = list(Path(output_directory).glob('graph.*.csv'))
        plt.subplot(2, 1, index)
        index += 1
        processes = []
        for graph_file in graph_files:
            process = int(
                    re.match(r"graph\.(\d+)\.csv", graph_file.name)
                    .group(1)
                    )
            processes.append(process)
            data = read_csv(graph_file, mode)
            plt.title(mode)
            plt.plot(data[0], data[1], label="Process " + str(process))
        plt.xlabel("Time step")
        plt.ylabel(mode + " count")
        total = [data / (max(processes)+1) for data in data[2]]
        plt.plot(data[0], total, label="Total / process count")
        plt.legend()
    plt.show()

if __name__ == "__main__":
    plot_graph(sys.argv[1])
