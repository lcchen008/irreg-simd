#!/bin/bash

#SBATCH -J simd-tiling
#SBATCH -o simd-tiling.%j
#SBATCH -t 2:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.128   ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.128   ../../input/45-3.0r/45-3.0r.xyz.reordered 
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.256   ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.256   ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.512   ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.512   ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.1024  ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.1024  ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.2048  ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.2048  ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.4096  ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.4096  ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.8192  ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.8192  ../../input/45-3.0r/45-3.0r.xyz.reordered
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.tiling.16384 ../../input/45-3.0r/45-3.0r.mesh.matlab.reordered.offset.16384 ../../input/45-3.0r/45-3.0r.xyz.reordered

#./main ../../input/32-3.0r/32-3.0r.mesh.matlab.reordered.tiling.16384 ../../input/32-3.0r/32-3.0r.mesh.matlab.reordered.offset.16384 ../../input/32-3.0r/32-3.0r.xyz.reordered

#./main ../../input/32-3.0r/32-3.0r.mesh.matlab.tiling ../../input/32-3.0r/32-3.0r.mesh.matlab.offset ../../input/32-3.0r/32-3.0r.xyz
# ibrun ./main ../../input/32-4.0r/32-4.0r.mesh.matlab.reordered ../../input/32-4.0r/32-4.0r.xyz.reordered
# ibrun ./main ../input/64-1.0r/64-1.5r.mesh.matlab.reordered ../input/64-1.0r/64-1.5r.xyz.reordered
