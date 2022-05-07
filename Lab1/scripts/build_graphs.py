#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import subprocess
import json

EXECUTABLE_PATH = "../../build/Lab1/lab1"

def closure_multi_time(threads, iterations):
    def func(size):
        child = subprocess.run([EXECUTABLE_PATH,
                                "--size", str(size),
                                "--iterations", str(iterations),
                                "--threads", str(threads)],
                               stdout=subprocess.PIPE)
        result = json.loads(child.stdout)
        return result["average"]
    return func

def closure_multi_pool_time(threads, iterations):
    def func(size):
        child = subprocess.run([EXECUTABLE_PATH,
                                "--size", str(size),
                                "--iterations", str(iterations),
                                "--threads", str(threads),
                                "--pool"],
                               stdout=subprocess.PIPE)
        result = json.loads(child.stdout)
        return result["average"]
    return func


def closure_single_time(iterations):
    def single_time(size):
        child = subprocess.run([EXECUTABLE_PATH,
                                "--size", str(size),
                                "--iterations", str(iterations)],
                               stdout=subprocess.PIPE)
        result = json.loads(child.stdout)
        return result["average"]
    return single_time

def annot_max(x,y, ax=None):
    xmax = x[np.argmax(y)]
    ymax = y.max()
    text= "time={:.3f}".format(ymax)
    if not ax:
        ax=plt.gca()
    ax.annotate(text, xy=(xmax, ymax), xytext=(xmax+.5,ymax+5))


if __name__ == "__main__":
    size = np.linspace(100, 20000, 10).astype(int)

    ITERATIONS = 3

    single = np.vectorize(closure_single_time(ITERATIONS))(size)

    fig, axes = plt.subplots(8, 2)

    plt.xlabel('matrix size')
    plt.ylabel('time')
    for i, ax in enumerate([a for t in axes for a in t]):
        THREADS = i + 1
        multi = np.vectorize(closure_multi_time(THREADS, ITERATIONS))(size)
        multi_pool = np.vectorize(
            closure_multi_pool_time(THREADS, ITERATIONS))(size)
        ax.plot(size, multi, label='multi')
        ax.plot(size, multi_pool, label='multi_pool')
        ax.plot(size, single, label='single')
        annot_max(size, multi, ax)
        annot_max(size, multi_pool, ax)
        annot_max(size, single, ax)
        ax.legend()
        ax.set_title("Threads: {}".format(THREADS))

    plt.show()
