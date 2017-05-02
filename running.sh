#!/bin/sh
#SBATCH --time=30
#SBATCH --partition=medium
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mcnelk@rpi.edu

export BGLOCKLESSMPIO_F_TYPE=0x47504653
srun --ntasks 18 -N 256 --overcommit -o logs/log.ranks18-threads4.log ./main.out 4

srun --ntasks 108 -N 256 --overcommit -o logs/log.ranks108-threads4.log ./main.out 4

srun --ntasks 628 -N 256 --overcommit -o logs/log.ranks628-threads4.log ./main.out 4

srun --ntasks 1296 -N 256 --overcommit -o logs/log.ranks1296-threads4.log ./main.out 4

srun --ntasks 18 -N 256 --overcommit -o logs/log.ranks18-threads8.log ./main.out 8

srun --ntasks 108 -N 256 --overcommit -o logs/log.ranks108-threads8.log ./main.out 8

srun --ntasks 628 -N 256 --overcommit -o logs/log.ranks628-threads8.log ./main.out 8

srun --ntasks 1296 -N 256 --overcommit -o logs/log.ranks1296-threads8.log ./main.out 8

