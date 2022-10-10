#!/bin/bash
#SBATCH --time=4-00:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 64
#SBATCH --mem 0
#SBATCH --partition=long


# Initialization
#cd results
#rm *.json
#cd ..
#cd missing_inputs
#rm *.txt
#cd ..
#rm -rf results_1; rm -rf results_2; rm -rf results_3


# Index the contents of the dir with application and platform models into a .txt
cd JSON/Inputs/Inputs02
ls -d "$PWD"/* > inputs_2.txt
mv inputs_2.txt /work/ws-tmp/cl603105-new_workspace/SNAP_and_Gen/NewGA/JSON/
cd /work/ws-tmp/cl603105-new_workspace/SNAP_and_Gen/NewGA/


# Parallel execution of the scheduler - GNU Parallel Shell Tool
#cat JSON/applpy_$j.txt|parallel -j 9 ./scheduler JSON/applpy/applpy_$j/{}
cat JSON/inputs_2.txt|xargs -n 1 -P 64 -t ./range



