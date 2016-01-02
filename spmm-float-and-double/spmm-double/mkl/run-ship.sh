#!/bin/bash
scp main mic0:/tmp
ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/shipsec5/shipsec5.mtx 14000000
