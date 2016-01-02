#!/bin/bash

#SBATCH -J euler
#SBATCH -o euler.%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

echo "Running on MIC and CPU"
export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#./main ../../input/euler/mesh.matlab.reordered.tiling ../../input/euler/mesh.matlab.reordered.offset ../../input/euler/mesh.xyz.reordered
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.128   ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.128 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.256   ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.256 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.512   ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.512 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.1024  ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.1024 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.2048  ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.2048 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.4096  ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.4096 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.8192  ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.8192 ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.tiling.16384 ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.offset.16384 ../../input/gsm_106857/gsm_106857.mtx.xyz 

./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.128    ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.128 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.256    ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.256 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.512    ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.512 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.1024   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.1024 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.2048   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.2048 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.4096   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.4096 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.8192   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.8192 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.tiling.16384  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.offset.16384 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.tiling.16384  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.offset.16384 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
