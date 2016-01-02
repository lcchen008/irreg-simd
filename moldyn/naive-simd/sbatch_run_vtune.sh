#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#ibrun ./main
amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/32-3.0r" ./main ../../input/32-3.0r/32-3.0r.mesh ../../input/32-3.0r/32-3.0r.xyz
amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/45-3.0r" ./main ../../input/45-3.0r/45-3.0r.mesh ../../input/45-3.0r/45-3.0r.xyz
