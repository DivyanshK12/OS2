import os
import re
import time

squares = [i*i for i in range(3, 11)] # list of squares from 9 to 100
threads = [2**i for i in range(1, 7)] # list of threads from 2 to 64

# task 1
# vary square count, keep thread count at 16
results = {}
with open("task2.txt", "a") as f:
    for square in threads:
        with open("input.txt", "w") as g:
            g.write(f"{square} 25\n")
        os.system(f"python3 sudoku_gen.py 25 >> input.txt")

        os.system("./main > output.txt")
        time.sleep(5) # wait for 5 seconds

        with open("output.txt", "r") as g:
            for line in g:
                a = re.match("Time taken by function: (\d+) microseconds", line)
                if a:
                    results[square] = a.groups()[0]

    writestr = ""
    for r in results.values():
        writestr += r + ","
    writestr = writestr[:-1] + "\n"
    f.write(writestr)
    # run code here, wait time can be 5 seconds I guess