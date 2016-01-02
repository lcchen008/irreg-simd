#!/bin/bash

#SBATCH -J seq-tiling 
#SBATCH -o seq-tiling.%j
#SBATCH -t 00:25:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#./main ../../input/32-3.0r/32-3.0r.mesh.matlab.reordered.seq.tiling ../../input/32-3.0r/32-3.0r.xyz.reordered
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.128   ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.256   ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.512   ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.1024  ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.2048  ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.4096  ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.8192  ../../input/gsm_106857/gsm_106857.mtx.xyz 
#./main ../../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered.seq.tiling.16384 ../../input/gsm_106857/gsm_106857.mtx.xyz 

./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.128   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.256   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.512   ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.1024  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.2048  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.4096  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.8192  ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
./main ../../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered.seq.tiling.16384 ../../input/kron_g500-logn19/kron_g500-logn19.mtx.xyz 
