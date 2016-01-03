#!/bin/bash

#SBATCH -J spmm-naive-simd 
#SBATCH -o spmm-naive-simd.%j
#SBATCH -t 00:30:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

# ./main ../datasets/appu/appu.mtx 20000
# ./main ../datasets/598a/598a.mtx 2000
# ./main ../datasets/msc10848/msc10848.mtx 20000
# ./main ../datasets/conf5_4-8x8-05/conf5_4-8x8-05.mtx 2500
# ./main ../datasets/shipsec5/shipsec5.mtx 1000

./main ../datasets/crankseg_1/crankseg_1.mtx 10000
