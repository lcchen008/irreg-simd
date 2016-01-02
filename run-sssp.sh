#!/bin/bash

#SBATCH -J SSSP 
#SBATCH -o SSSP.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic

SECONDS=0

################## Executions ###################
  # Build binaries:
  echo -e "\e[0;32m **** Building binaries... **** \e[0m"
  cd sssp/seq-sssp
  make clean
  make
  cd ../simd-sssp
  make clean
  make
  cd ../mimd-sssp
  make clean
  make

  # Run on higgs-twitter:
  echo -e "\e[0;34m ************************ Running SSSP on higgs-twitter ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-sssp
    ./sssp ../../datasets/twitter/higgs-twitter-orig
    
    echo -e "\e[0;32m **** Running [Serial Tiling] **** \e[0m"
    ./sssp ../../datasets/twitter/higgs-twitter-1024 1024

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../simd-sssp
    ./sssp ../../datasets/twitter/higgs-twitter-orig 0

    # Our approach
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
    ./run_twitter.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [MIMD Tiling (Our)]  **** \e[0m"
    cd ../mimd-sssp
    ./run_twitter.sh

    # Run on soc-Pokec:
  echo -e "\e[0;34m ************************ Running SSSP on soc-Pokec ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-sssp
    ./sssp ../../datasets/pokec/soc-pokec-orig

    echo -e "\e[0;32m **** Running [Serial Tiling] **** \e[0m"
    ./sssp ../../datasets/pokec/soc-pokec-1024 1024
    
    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../simd-sssp
    ./sssp ../../datasets/pokec/soc-pokec-orig 0

    # Our approach
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
    ./run_pokec.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [MIMD Tiling (Our)]  **** \e[0m"
    cd ../mimd-sssp
    ./run_pokec.sh

    cd ../..

    
echo -e "\e[0;32m **** SSSP finished! Total time elapsed: $SECONDS secs ... **** \e[0m"
