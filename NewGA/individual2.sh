#!/bin/bash
#SBATCH --time=00:10:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 1
#SBATCH --mem 0
#SBATCH --partition=short

./individual2 example_N1.json

