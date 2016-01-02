#!/bin/bash
scp main mic0:/tmp
ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/crankseg_1/crankseg_1.mtx 24000000
