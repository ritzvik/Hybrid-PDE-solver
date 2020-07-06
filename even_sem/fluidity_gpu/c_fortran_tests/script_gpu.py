import subprocess
import time
import sys

dimensions = [100 * (2 ** i) for i in range(0, 15)]

fw = open(sys.argv[1], "w")

result = subprocess.run(["make", "ex3"], stdout=subprocess.PIPE)
result = subprocess.run(["make", "ex3f"], stdout=subprocess.PIPE)
times = list()

for dimension in dimensions:
    times.append(list())
    print("\n~~~~~~~{}~~~~~~~\n".format(dimension))
    fw.write(str(dimension) + " ")
    for ex in ["./ex3", "./ex3f"]:
        start_time = time.time()
        result = subprocess.run(
            [
                ex,
                "-vec_type",
                "cuda",
                "-mat_type",
                "aijcusparse",
                "-n",
                "{}".format(dimension),
            ],
            stdout=subprocess.PIPE,
        )
        times[-1].append(time.time() - start_time)
        print(times[-1][-1])
        fw.write(str(times[-1][-1]) + " ")
    fw.write("\n")
