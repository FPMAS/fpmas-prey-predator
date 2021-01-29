import pathlib
import random
import shutil
import sys

if __name__ == "__main__":
    root = pathlib.Path(sys.argv[1])
    if root.exists():
        shutil.rmtree(root)
    for config in ["set1", "set2"]:
        for num_proc in ["2", "4", "8", "16", "32", "64"]:
            for job in [str(random.randint(300000, 400000)) for i in range(1)]:
                path = root / config / num_proc / job
                path.mkdir(parents=True)
                time_file = path / "time.txt"
                with open(time_file, 'w') as time:
                    random_value = 1000 / int(num_proc)
                    random_value += random.uniform(-10, 10)
                    print(str(random_value), file=time)

