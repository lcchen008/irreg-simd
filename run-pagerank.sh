#!/bin/bash

#SBATCH -J PAGERANK
#SBATCH -o PAGERANK.o%j
#SBATCH -t 02:00:00
#SBATCH -n1 -N1
#SBATCH -p normal-mic

SECONDS=0

################## Executions ###################
  # Build binaries:
  echo -e "\e[0;32m **** Building binaries... **** \e[0m"
  cd page_rank/seq-page_rank
  make clean
  make
  cd ../simd-page_rank
  make clean
  make
  cd ../mimd-page_rank
  make clean
  make

  # Run on higgs-twitter:
  echo -e "\e[0;34m ************************ Running Page_rank on higgs-twitter ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-page_rank
    ./page_rank ../../datasets/twitter/higgs-twitter-orig
    
    echo -e "\e[0;32m **** Running [Serial Tiling] **** \e[0m"
    ./page_rank ../../datasets/twitter/higgs-twitter-1024 1024

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../simd-page_rank
    ./page_rank ../../datasets/twitter/higgs-twitter-orig 0

    # Our approach
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
    ./run_twitter.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [MIMD Tiling (Our)]  **** \e[0m"
    cd ../mimd-page_rank
    ./run_twitter.sh

    # Run on soc-Pokec:
  echo -e "\e[0;34m ************************ Running Page_rank on soc-Pokec ************************** \e[0m"
    # Serial version, base line: 
    echo -e "\e[0;32m **** Running [Serial] **** \e[0m"
    cd ../seq-page_rank
    ./page_rank ../../datasets/pokec/soc-pokec-orig
    
    echo -e "\e[0;32m **** Running [Serial Tiling] **** \e[0m"
    ./page_rank ../../datasets/pokec/soc-pokec-1024 1024

    # Naive SIMD: 
    echo -e "\e[0;32m **** Running [SIMD Naive] **** \e[0m"
    cd ../simd-page_rank
    ./page_rank ../../datasets/pokec/soc-pokec-orig 0

    # Our approach
    echo -e "\e[0;32m **** Running [SIMD Tiling (Our)] **** \e[0m"
    ./run_pokec.sh

    # Our approach:
    echo -e "\e[0;32m **** Running [MIMD Tiling (Our)]  **** \e[0m"
    cd ../mimd-page_rank
    ./run_pokec.sh

echo -e "\e[0;32m **** pagerank finished! Total time elapsed: $SECONDS secs ... **** \e[0m"
