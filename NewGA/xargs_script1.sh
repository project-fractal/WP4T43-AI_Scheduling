#!/bin/bash
#SBATCH --time=4-00:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 64
#SBATCH --mem 0
#SBATCH --partition=long


# Index the contents of the dir with application and platform models into a .txt
cd JSON/Inputs/Inputs01
ls -d "$PWD"/* > inputs_1.txt
mv inputs_1.txt /work/ws-tmp/cl603105-new_workspace/AI_scheduling_components/NewGA/JSON/
cd /work/ws-tmp/cl603105-new_workspace/AI_scheduling_components/NewGA/


# Parallel execution of the scheduler - GNU Parallel Shell Tool
#cat JSON/applpy_$j.txt|parallel -j 9 ./scheduler JSON/applpy/applpy_$j/{}
cat JSON/inputs_1.txt|xargs -n 1 -P 64 -t ./GA
