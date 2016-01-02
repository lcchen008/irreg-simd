#!/usr/bin/python
#tuple to adjacency list

fin=open("", "r")
fout=open("pokec/soc-pokec-relationships.list", "w")
previous = 1
for line in fin:
  [a, b] = line.split()
  while (int(a) != previous):
      fout.write("\n")
      previous = previous + 1
  fout.write(" " + b)

