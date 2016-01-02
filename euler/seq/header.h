#ifndef HEADER_H
#define HEADER_H

#define Replicate
#define unstructured_BLOCKS 1
#define unstructured_THREADS 1

void unstructured_func(float *update,int edge[][2], int face[][4], float velocity[][3], float edgeData[][5], float faceData[][3], int numNodes, int numEdges, int numFaces);
void Free_func();

#endif
