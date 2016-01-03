#!/bin/bash

#SBATCH -J spmm-naive-simd 
#SBATCH -o spmm-naive-simd.%j
#SBATCH -t 00:30:00
#SBATCH -n1 -N1
#SBATCH -p development 

echo "Running on MIC and CPU"
export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

./main ../../datasets/msc10848/msc10848.mtx 10000
