import matplotlib.pyplot as plt
from pathlib import Path
import sys

'''
The output directory structure is as follow:
    - <root>
        - <config>
            - <num_proc>
                - <job_id>
                    - model.csv
                    - time.txt
                    - graph.*.csv
'''

def read_data(output_directory):
    output_path = Path(output_directory)
    configs = [config_dir for config_dir in output_path.iterdir()]

    data = {}
    for config in configs:
        num_procs = [num_proc for num_proc in config.iterdir()]
        data[config.name] = [[], []]
        for num_proc in sorted([int(num_proc.name) for num_proc in num_procs]):
            data[config.name][0].append(num_proc)
            jobs = [job for job in (config / str(num_proc)).iterdir()]
            time = 0.0
            for job in jobs:
                with open(job / "time.txt", "r") as time_file:
                    time += float(time_file.read())
            data[config.name][1].append(time / len(jobs))
    return data

def plot_speedup(data):
    print(data)
    plt.title("Execution of the Prey-Predator model (HardSyncMode)")
    for config in data.keys():
        plt.plot(data[config][0], data[config][1], label=config)
    plt.legend()
    plt.xlabel("Number of cores")
    plt.ylabel("Execution time (seconds)")
    plt.show()

if __name__ == "__main__":
    plot_speedup(read_data(sys.argv[1]))
        
