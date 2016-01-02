#!/usr/bin/python
# Filename: reorder.py
# Usage: python reorder.py meshfile coordinate file

import os
import subprocess
import sys
import time

metis_parts = 2048

#if len(sys.argv) != 3:
#  print "Input must have two args, the mesh file and the coordinates file"
#  sys.exit()


from time import time
start=time()
print "Original graph input: ", sys.argv[1]
if (len(sys.argv) > 2):
        print "Original node input: ", sys.argv[2]
print "***Converting to undirected graph, and generating matlab input file in Matrix Market format...***"
os.system('./tdu ' + sys.argv[1])

print "Using metis to reorder..."
os.system("/work/02687/binren/graph/tools/software/metis/bin/gpmetis " + sys.argv[1] + ".metis " + str(metis_parts) + " -minconn")
print "Gen ordering file from the metis result..."
os.system("./gof " + str(metis_parts) + " " + sys.argv[1] + ".metis.part." + str(metis_parts))
print "Reorder mesh input (moldyn graph input) based on the new ordering..."
os.system("./rmed " + sys.argv[1] + " " + sys.argv[1] + ".metis.part." + str(metis_parts) + ".ordering");
print "Reorder matlab input (moldyn graph input) based on the new ordering..."
os.system("./rmd " + sys.argv[1] + ".matlab " + sys.argv[1] + ".metis.part." + str(metis_parts) + ".ordering");
if (len(sys.argv) > 2):
  print "Reorder coordinate data (moldyn input) based on the new ordering..."
  os.system("./rc " + sys.argv[2] + " " +  sys.argv[1] + ".metis.part." + str(metis_parts) + ".ordering");
finish=time()
print "Finsh."
print "OUTPUT: " + sys.argv[1] + ".matlab.reordered"
if (len(sys.argv) > 2):
  print "AND " + sys.argv[2] + ".reordered"
print "Total processing time: " + str(finish-start) + " seconds."
