#!/usr/bin/env python

import numpy as np
import pandas as pd
import sys
import os
import argparse

################################################################################
# Usage:
################################################################################
# usage: print_advisor.py [-h] -p PROJECT_DIR 
#
#optional arguments:
#  -h, --help            show this help message and exit
#  -p PROJECT_DIR, --project-dir PROJECT_DIR 
#                        project-dir name
###
#
# Example:
#
#    python print_advisor.py -p matrix_advisor-2-4-8192x8192
################################################################################

# Add advisor dir to sys.path
sys.path.append(os.environ['ADVISOR_2020_DIR']+'/pythonapi/')

import advisor

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--project-dir", required=True, help="project-dir name")
args = parser.parse_args()

variables = ['self_ai', 'self_gflops', 'self_gflop', 'self_time', 'self_dram_gb', 'self_dram_loaded_gb', 'self_dram_stored_gb', 'self_memory_gb', 'self_loaded_gb', 'self_stored_gb', 'self_l2_gb', 'self_l3_gb', 'loop_name']

project = advisor.open_project(args.project_dir)
data = project.load(advisor.SURVEY)
rows = [{col: row[col] for col in row} for row in data.bottomup]
df = pd.DataFrame(rows).replace('', np.nan)

for var in variables:
    if 'loop_name' not in var:
        df[var] = df[var].astype(float)

print(df[variables].dropna().sort_values(by=['self_dram_gb'],ascending=False).head(5))

