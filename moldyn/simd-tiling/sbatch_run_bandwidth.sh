#!/bin/bash

#SBATCH -J mymat
#SBATCH -o mymat.o%j
#SBATCH -t 00:35:00
#SBATCH -n1 -N1
#SBATCH -p development 

module load vtune

export INTEL_LICENSE_FILE=/home1/02687/binren/intel/licenses/intel.current.20170224.lic
export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

amplxe-cl -collect knc-bandwidth -r "../vtune10" -target-duration-type=long ./main ../../input/32-3.0r/32-3.0r.mesh.matlab ../../input/32-3.0r/32-3.0r.xyz
