#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 00:25:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic 

echo "Running on MIC and CPU"
export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

# ibrun ./main < moldyn.in32.40 
ibrun ./main < moldyn.in45 
# ibrun ./main ../input/64-1.0r/64-1.5r.mesh.matlab.reordered ../input/64-1.0r/64-1.5r.xyz.reordered
