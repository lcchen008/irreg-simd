#!/bin/bash

#SBATCH -J Moldyn
#SBATCH -o Moldyn.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic

SECOND=0

################## Executions ###################
  # Build binaries:
  echo -e "\e[0;32m **** Building binaries... **** \e[0m"
  cd moldyn/seq-float
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

  # Run on data 32-3.0r:
  echo -e "\e[0;34m ************************ Running Moldyn on 32-3.0r ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-float
    ./main < moldyn.in32
    
    # Serial version on tiled input: 
    echo -e "\e[0;32m **** Running [Serial Tiling] (Running with different tile sizes) **** \e[0m"
    cd ../seq-tiling
    ./run32-3.0r.sh

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../naive-simd
    ./run32-3.0r.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] (Running with different tile sizes and different thread numbers) **** \e[0m"
    cd ../simd-tiling
    ./run32-3.0r.sh

  # Run on data 45-3.0r:
  echo -e "\e[0;34m ************************ Running Moldyn on 45-3.0r ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-float
    ./main < moldyn.in45
    
    # Serial version on tiled input: 
    echo -e "\e[0;32m **** Running [Serial Tiling] (Running with different tile sizes) **** \e[0m"
    cd ../seq-tiling
    ./run45-3.0r.sh

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../naive-simd
    ./run45-3.0r.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] (Running with different tile sizes and different thread numbers) **** \e[0m"
    cd ../simd-tiling
    ./run45-3.0r.sh

  cd ../..
  # Now at outer most directory.

echo -e "\e[0;32m **** Moldyn finished! Total time elapsed: $SECONDS secs ... **** \e[0m"
