#!/usr/bin/env python

import numpy as np
import sys
import struct
import os
import itertools
import argparse

################################################################################
# Usage:
################################################################################
# usage: print_counter.py [-h] -f FILENAME -N NODES
#
# optional arguments:
#  -h, --help            show this help message and exit
#  -f FILENAME, --filename FILENAME
#                        name of the binary file
#  -N NODES, --nodes NODES
#                        number of nodes
###
#
# Example:
#
#    python print_counters.py -f outputs/matrix_add_aries-8-8192-32.counters.0.bin -N 8
################################################################################


def my_range(start, end, step):
    while start <= end:
        yield start
        start += step


parser = argparse.ArgumentParser()
parser.add_argument("-f", "--filename", required=True, help="name of the binary file")
parser.add_argument("-N", "--nodes", required=True, help="number of nodes")
args = parser.parse_args()

try:
    f = open("counters.txt", "r")
except IOError:
    print >> sys.stderr, "Could not open file counters.txt"
    sys.exit(1)
lines = f.readlines()
f.close()

counters = []
for i in range(len(lines)):
    counters.append(lines[i].split("\n")[0])

n = int(args.nodes)

filename = args.filename
try:
    binary_file = open(filename, "rb")
except IOError:
    print >> sys.stderr, "Could not open file '" + filename + "'"
    sys.exit(1)

values = {}
for i in range(len(counters)):
    values[counters[i]] = []

file_length_in_bytes = os.path.getsize(filename)
for i in range(n):
    c = 0
    for j in my_range(
        (file_length_in_bytes / n) * i + 1, (file_length_in_bytes / n) * (i + 1), 8
    ):
        couple_bytes = binary_file.read(8)
        values[counters[c]].append(struct.unpack("q", couple_bytes))
        c = c + 1
binary_file.close()

filename = args.filename.split(".")[0] + ".timings.json"
try:
    f = open(filename, "r")
except IOError:
    print >> sys.stderr, "Could not open file '" + filename + "'"
    sys.exit(1)
lines = f.readlines()
f.close()

timings = []  # nanoseconds
for lin in range(len(lines)):
    if '"time":' in lines[lin]:
        for i in range(n):
            if i == n - 1:
                timings.append(
                    float(lines[lin].split("[")[1].split(", ")[i].split("]")[0])
                )
            else:
                timings.append(float(lines[lin].split("[")[1].split(", ")[i]))

names = [
    "AR_NIC_ORB_PRF_RSP_BYTES_RCVD",
    "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_FLITS",
    "AR_NIC_RSPMON_NPT_EVENT_CNTR_NL_PKTS",
]

z = [
    (i - j) * 16
    for i, j in zip(
        list(itertools.chain(*values[names[1]])),
        list(itertools.chain(*values[names[2]])),
    )
]  # SEND bytes
print "SEND bytes: " + str(z)
print "RCVD bytes: " + str(values[names[0]])
