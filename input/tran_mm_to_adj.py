#!/usr/bin/python
#matrix market to adjacency list
#input is 1 based
#output is also 1 based

import sys
import time

if len(sys.argv) != 2:
  print "Must indicate the matrix market file..."
  sys.exit()

fin=open(sys.argv[1], "r")
fout=open(sys.argv[1] + ".mesh", "w")
xyzfout=open(sys.argv[1] + ".xyz", "w")

# Skip comments
first_line = fin.readline()
while first_line[0] == '%':
  first_line = fin.readline()

[num_rows, num_cols, nnz] = first_line.split()

# Allocate result data structure
adjs = []
for i in range(int(num_rows)):
  adjs.append([])

print "size of adjs: " + str(len(adjs))

# Construct result data
for line in fin:
  [row, col, val] = line.split()
  adjs[int(row) - 1].append(col)

# Write result
# Write info
fout.write(num_rows)
fout.write(" ")
fout.write(nnz)
for l in adjs:
  fout.write("\n")
  for ll in l:
    fout.write(ll)
    fout.write(" ")

#Write XYZ file
for i in range(int(num_rows)):
    xyzfout.write("0.355 0 0\n")
