#!/usr/bin/env python

import subprocess
import sys
import glob
import os

def main():
    get_traces = lambda : glob.glob("swimps_trace_swimps-dummy*")

    for trace in get_traces():
        os.remove(trace)

    completed_process = subprocess.run(sys.argv[1:])
    if completed_process.returncode != 0:
        return completed_process.returncode

    traces = get_traces()
    if len(traces) != 1:
        return -1

    return 0

if __name__ == "__main__":
    sys.exit(main());

