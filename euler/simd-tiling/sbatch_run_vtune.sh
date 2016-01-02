#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/
APP=euler
INPUT=gsm_106857

COUNTER=128
while [ $COUNTER -lt 32768 ]; do
  amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/$APP/$INPUT-$COUNTER" ./main ../../input/$INPUT/$INPUT.mtx.mesh.matlab.tiling.$COUNTER  ../../input/$INPUT/$INPUT.mtx.mesh.matlab.offset.$COUNTER  ../../input/$INPUT/$INPUT.mtx.xyz
  let COUNTER=COUNTER*2
done

INPUT=kron_g500-logn19

COUNTER=128
while [ $COUNTER -lt 32768 ]; do
  amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/$APP/$INPUT-$COUNTER" ./main ../../input/$INPUT/$INPUT.mtx.mesh.matlab.tiling.$COUNTER  ../../input/$INPUT/$INPUT.mtx.mesh.matlab.offset.$COUNTER  ../../input/$INPUT/$INPUT.mtx.xyz
  let COUNTER=COUNTER*2
done
