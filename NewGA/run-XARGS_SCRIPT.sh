#!/bin/bash
#SBATCH --time=0-2:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 64
#SBATCH --mem 0
#SBATCH --partition=medium

len=$(ls | grep  "xargs_script*" | wc -l)
for i in $( seq 1 $len )
do  
	echo "Executing xargs_script$i.sh"
        sbatch xargs_script$i.sh
        echo "Execution Complete of xargs_script$i.sh"
done
