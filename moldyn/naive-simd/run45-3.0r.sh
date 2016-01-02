#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 00:25:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic 

./main ../../input/45-3.0r/45-3.0r.mesh ../../input/45-3.0r/45-3.0r.xyz
