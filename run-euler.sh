#!/bin/bash

#SBATCH -J Euler 
#SBATCH -o Euler.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic

SECOND=0

################## Executions ###################
  # Build binaries:
  echo -e "\e[0;32m **** Building binaries... **** \e[0m"
  cd euler/seq
  make clean
  make
  cd ../seq-tiling
  make clean
  make
  cd ../naive-simd
  make clean
  make
  cd ../simd-tiling
  make clean
  make

  # Run on data kron_g500-logn19:
  echo -e "\e[0;34m ************************ Running Euler on kron_g500-logn19 ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq
    ./run-kron.sh 
    
    # Serial version on tiled input: 
    echo -e "\e[0;32m **** Running [Serial Tiling] (Running with different tile sizes) **** \e[0m"
    cd ../seq-tiling
    ./run-kron.sh

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../naive-simd
    ./run-kron.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] (Running with different tile sizes and different thread numbers) **** \e[0m"
    cd ../simd-tiling
    ./run-kron.sh

  # Run on data gsm_106857:
  echo -e "\e[0;34m ************************ Running Euler on gsm_106857 ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq
    ./run-gsm.sh
    
    # Serial version on tiled input: 
    echo -e "\e[0;32m **** Running [Serial Tiling] (Running with different tile sizes) **** \e[0m"
    cd ../seq-tiling
    ./run-gsm.sh

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../naive-simd
    ./run-gsm.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] (Running with different tile sizes and different thread numbers) **** \e[0m"
    cd ../simd-tiling
    ./run-gsm.sh

  cd ../..
  # Now at outer most directory.


echo -e "\e[0;32m **** Euler finished! Total time elapsed: $SECONDS secs ... **** \e[0m"
