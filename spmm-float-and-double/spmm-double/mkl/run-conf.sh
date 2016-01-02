#!/bin/bash
scp main mic0:/tmp
ssh mic0 LD_LIBRARY_PATH=/tmp /tmp/main /work/02687/binren/linchuan/spmm/datasets/conf5_4-8x8-05/conf5_4-8x8-05.mtx 11000000
