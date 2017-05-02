#!/bin/sh
echo "#!/bin/sh" > running.sh
echo "#SBATCH --time=30" >> running.sh
echo "#SBATCH --partition=medium" >> running.sh
echo "#SBATCH --mail-type=ALL" >> running.sh
echo "#SBATCH --mail-user=mcnelk@rpi.edu" >> running.sh
echo "" >> running.sh
echo "export BGLOCKLESSMPIO_F_TYPE=0x47504653" >> running.sh

for threads in 4 8
do
  for tasks in 18 108 628 1296
  do
    echo "srun --ntasks $tasks -N 256 --overcommit -o logs/log.ranks$tasks-threads$threads.log ./main.out $threads" >> running.sh
    echo "" >> running.sh
  done
done

echo "Finished"
