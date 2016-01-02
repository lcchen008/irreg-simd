#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 00:15:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#ibrun ./main
amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune1" ./main ../../input/32-3.0r/32-3.0r.mesh.matlab ../../input/32-3.0r/32-3.0r.xyz
