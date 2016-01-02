#!/bin/bash

#SBATCH -J output_tile 
#SBATCH -o output_tile.%j
#SBATCH -t 2:00:00
#SBATCH -n1 -N1
#SBATCH -p development 

export MIC_LD_LIBRARY_PATH=$MIC_LD_LIBRARY_PATH:/opt/apps/intel/13/composer_xe_2013.2.146/compiler/lib/mic/

echo -e "\e[0;34m ************************ Tiling Moldyn Input ************************** \e[0m"
echo -e "\e[0;32m **** Tiling 32-3.0r... **** \e[0m"
./main ../input/32-3.0r/32-3.0r.mesh.matlab 1024 
./main ../input/32-3.0r/32-3.0r.mesh.matlab 2048 
./main ../input/32-3.0r/32-3.0r.mesh.matlab 4096 
./main ../input/32-3.0r/32-3.0r.mesh.matlab 8192 

echo -e "\e[0;32m **** Tiling 45-3.0r... **** \e[0m"
./main ../input/45-3.0r/45-3.0r.mesh.matlab 1024 
./main ../input/45-3.0r/45-3.0r.mesh.matlab 2048 
./main ../input/45-3.0r/45-3.0r.mesh.matlab 4096 
./main ../input/45-3.0r/45-3.0r.mesh.matlab 8192 

echo -e "\e[0;34m ************************ Tiling Euler Input ************************** \e[0m"
echo -e "\e[0;32m **** Tiling gsm_106857... **** \e[0m"
./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab 1024 
./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab 2048 
./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab 4096 
./main ../input/gsm_106857/gsm_106857.mtx.mesh.matlab 8192 

echo -e "\e[0;32m **** Tiling kron_g500-logn19... **** \e[0m"
./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab 1024 
./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab 2048 
./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab 4096 
./main ../input/kron_g500-logn19/kron_g500-logn19.mtx.mesh.matlab 8192 
