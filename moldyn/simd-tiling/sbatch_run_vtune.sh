#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/
APP=moldyn
INPUT=45-3.0r

COUNTER=128
while [ $COUNTER -lt 32768 ]; do
  amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/$APP/$INPUT-$COUNTER" ./main ../../input/$INPUT/$INPUT.mesh.matlab.tiling.$COUNTER  ../../input/$INPUT/$INPUT.mesh.matlab.offset.$COUNTER  ../../input/$INPUT/$INPUT.xyz
  let COUNTER=COUNTER*2
done

#amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/$APP/32-3.0r-128" ./main ../../input/32-3.0r/32-3.0r.mesh.matlab.tiling.128  ../../input/32-3.0r/32-3.0r.mesh.matlab.offset.128  ../../input/32-3.0r/32-3.0r.xyz

#amplxe-cl -collect knc-general-exploration -knob enable-vpu-metrics=true -knob enable-tlb-metrics=true -r "../vtune/moldyn/32-3.0r-128" ./main ../../input/32-3.0r/32-3.0r.mesh.matlab.tiling.128  ../../input/32-3.0r/32-3.0r.mesh.matlab.offset.128  ../../input/32-3.0r/32-3.0r.xyz
