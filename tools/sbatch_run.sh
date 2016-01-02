#!/bin/bash

#SBATCH -J output_tile 
#SBATCH -o output_tile.%j
#SBATCH -t 2:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

#./main ../input/euler/mesh.matlab.reordered
./main ../input/32-3.0r/32-3.0r.mesh.matlab 8192 

#./main ../input/45-3.0r/45-3.0r.mesh.matlab 128
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 256 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 512 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 1024 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 2048 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 4096 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 8192 
#./main ../input/45-3.0r/45-3.0r.mesh.matlab 16384 

#./main ../input/gsm_106857/gsm_106857.mtx 128 
#./main ../input/gsm_106857/gsm_106857.mtx 256 
#./main ../input/gsm_106857/gsm_106857.mtx 512 
#./main ../input/gsm_106857/gsm_106857.mtx 1024 
#./main ../input/gsm_106857/gsm_106857.mtx 2048 
#./main ../input/gsm_106857/gsm_106857.mtx 4096 
#./main ../input/gsm_106857/gsm_106857.mtx 8192 
#./main ../input/gsm_106857/gsm_106857.mtx 16384 

#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 128 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 256 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 512 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 1024 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 2048 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 4096 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 8192 
#./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab.reordered 16384 

#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 128 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 256 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 512 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 1024 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 2048 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 4096 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 8192 
#./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab.reordered 16384 

# ./main ../input/test/test.matlab
# ibrun ./main ../../input/32-4.0r/32-4.0r.mesh.matlab.reordered ../../input/32-4.0r/32-4.0r.xyz.reordered
# ibrun ./main ../input/64-1.0r/64-1.5r.mesh.matlab.reordered ../input/64-1.0r/64-1.5r.xyz.reordered
