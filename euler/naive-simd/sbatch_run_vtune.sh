#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#ibrun ./main
amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/euler/naive/gsm_106857" ./main ../../input/gsm_106857/gsm_106857.mtx.mesh ../../input/gsm_106857/gsm_106857.mtx.xyz
#amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/euler/naive/kron_g500-logn19" ./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz
