#!/bin/bash

#SBATCH -J simd-tiling
#SBATCH -o simd-tiling.%j
#SBATCH -t 2:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

./main ../../input/45-3.0r/45-3.0r.mesh.matlab.tiling.1024  ../../input/45-3.0r/45-3.0r.mesh.matlab.offset.1024  ../../input/45-3.0r/45-3.0r.xyz
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.tiling.2048  ../../input/45-3.0r/45-3.0r.mesh.matlab.offset.2048  ../../input/45-3.0r/45-3.0r.xyz
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.tiling.4096  ../../input/45-3.0r/45-3.0r.mesh.matlab.offset.4096  ../../input/45-3.0r/45-3.0r.xyz
./main ../../input/45-3.0r/45-3.0r.mesh.matlab.tiling.8192  ../../input/45-3.0r/45-3.0r.mesh.matlab.offset.8192  ../../input/45-3.0r/45-3.0r.xyz
