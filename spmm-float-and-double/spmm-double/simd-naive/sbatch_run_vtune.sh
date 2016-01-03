#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

INPUT=msc10848
NUM_BUCKETS=20000

#COUNTER=128
#while [ $COUNTER -lt 32768 ]; do
#  amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/$APP/$INPUT-$COUNTER" ./main ../../input/$INPUT/$INPUT.mesh.matlab.tiling.$COUNTER  ../../input/$INPUT/$INPUT.mesh.matlab.offset.$COUNTER  ../../input/$INPUT/$INPUT.xyz
#  let COUNTER=COUNTER*2
#done

amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/$INPUT" ./main ../datasets/$INPUT/$INPUT.mtx $NUM_BUCKETS 

INPUT=conf5_4-8x8-05
NUM_BUCKETS=2500

amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/$INPUT" ./main ../datasets/$INPUT/$INPUT.mtx $NUM_BUCKETS 

INPUT=shipsec5
NUM_BUCKETS=1000

amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/$INPUT" ./main ../datasets/$INPUT/$INPUT.mtx $NUM_BUCKETS 

INPUT=crankseg_1
NUM_BUCKETS=10000

amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/naive/$INPUT" ./main ../datasets/$INPUT/$INPUT.mtx $NUM_BUCKETS 
