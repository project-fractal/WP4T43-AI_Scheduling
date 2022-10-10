#!/bin/bash
#SBATCH --time=0-2:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 64
#SBATCH --mem 0
#SBATCH --partition=medium


currpath=$(pwd)

echo "1---Changing path to gen"
cd $currpath/scenario_generator

echo "2---Creating scenarios  - Scenario Generator component"
./sosgen

echo "3---Copying scenarios into $currpath/NewGA/JSON"
cp -r $currpath/scenario_generator/scenarios $currpath/NewGA/JSON/

echo "4---Moving split_files.sh to scenarios for splitting in to batches of Input folders"
cp -r $currpath/NewGA/JSON/split_files.sh $currpath/NewGA/JSON/scenarios/
cd $currpath/NewGA/JSON/scenarios/

echo "5---Running split_files.sh"
chmod 777 split_files.sh
sbatch split_files.sh

sleep 600

echo "6---Moved input file from Input26 to the Input folder where we found split_files.sh"
cd $currpath/NewGA/JSON/scenarios/
find . -name split_files.sh -printf $PWD/"%h\n" >> copy-files-split.txt
find . -name slurm*.out -printf $PWD/"%h\n" >> copy-files-slurm.txt
cd "$(head -1 copy-files-split.txt)"
splitpath=$(pwd)
cd ../Inputs26/
find . -maxdepth 1 -type f | head -1 | xargs mv -t $splitpath
cd ..
cd "$(head -1 copy-files-slurm.txt)"
slurmpath=$(pwd)
cd ../Inputs26/
find . -maxdepth 1 -type f | head -1 | xargs mv -t $slurmpath

echo "7---Deleting split file, slurm file and copy-files*"
cd $currpath/NewGA/JSON/scenarios/
pwd
cd "$(head -1 copy-files-split.txt)"
rm -rf split_files.sh
cd ..
cd "$(head -1 copy-files-slurm.txt)"
rm -rf slurm*
cd ..
rm -rf copy-files*

echo "8---Deleted Input26"
cd $currpath/NewGA/JSON/scenarios/
rm -rf Inputs26

echo "9---Change Name of folders"
counter=0
for directory in *
do  
	let counter++
	if [ $counter -lt 10 ]
	then
		mv "Inputs"$counter"" "Inputs0"$counter"" || echo 'Could not rename '"$directory"''
	fi
done

echo "10---Change name of Input folder"
cd $currpath/NewGA/JSON/ 
mv scenarios Inputs

echo "11---Changed path in all xargs_script.sh file"
cd $currpath/NewGA/
chmod 777 change_path.sh
./change_path.sh

echo "12---Making all"
make

echo "13---Running ./run-XARGS_SCRIPT.sh ---- Genetic Algorithm component"
chmod 777 run-XARGS_SCRIPT.sh
./run-XARGS_SCRIPT.sh

echo "14---Installing all required packages"
cd $currpath
./python-packages.sh

echo "15---Running machine learning algorithm - Machine Learning model component"
cd $currpath/NewGA/
python3 ANN_priorities_schedule.py
