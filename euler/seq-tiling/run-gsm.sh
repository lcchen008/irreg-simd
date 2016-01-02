#!/bin/bash

#SBATCH -J seq-tiling 
#SBATCH -o seq-tiling.%j
#SBATCH -t 00:25:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.seq.tiling.1024  ../../input/gsm_106857/gsm_106857.mtx.xyz 
./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.seq.tiling.2048  ../../input/gsm_106857/gsm_106857.mtx.xyz 
./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.seq.tiling.4096  ../../input/gsm_106857/gsm_106857.mtx.xyz 
./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.seq.tiling.8192  ../../input/gsm_106857/gsm_106857.mtx.xyz 
