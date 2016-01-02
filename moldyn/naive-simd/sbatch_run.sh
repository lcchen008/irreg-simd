#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 00:25:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#./main ../../input/32-3.0r/32-3.0r.mesh.reordered ../../input/32-3.0r/32-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh ../../input/45-3.0r/45-3.0r.xyz
# ibrun ./main ../../input/32-4.0r/32-4.0r.mesh.matlab.reordered ../../input/32-4.0r/32-4.0r.xyz.reordered
# ibrun ./main ../input/64-1.0r/64-1.5r.mesh.matlab.reordered ../input/64-1.0r/64-1.5r.xyz.reordered
