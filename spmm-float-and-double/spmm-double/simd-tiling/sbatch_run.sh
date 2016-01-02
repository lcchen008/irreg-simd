#!/bin/bash

#SBATCH -J spmm-tiling 
#SBATCH -o spmm-tiling.%j
#SBATCH -t 00:30:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export INTEL_LICENSE_FILE=/home1/02687/binren/intel/licenses/intel.current.20170224.lic
export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

# ./main ../datasets/msc10848/msc10848.mtx 1800
# ./main ../datasets/598a/598a.mtx 2000
# ./main ../datasets/shipsec5/shipsec5.mtx 400
./main ../datasets/crankseg_1/crankseg_1.mtx 2000
#amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune3" ./main 

#./main ../datasets/conf5_4-8x8-05/conf5_4-8x8-05.mtx 2000
