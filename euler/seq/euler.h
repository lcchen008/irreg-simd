#ifndef _UNSTRUCTURED
#define _UNSTRUCTURED

const int MAX_NODES=1500000;
const int MAX_EDGES=9000000;
const int MAX_FACES=1684000;
const int MAX_BOUNDARY_NODES=275000;
const int MAX_BOUNDARY_TYPES=10;
const int TYPE_TAN=3;
const int TYPE_IO=6;
const int START=1;
const int END=2;
const int SIZE=3;
const float DEFAULT_BND_EDGE_DATA=0.5;

int numBoundaryNodes, numBoundaryEdges;
int boundaryFacePtr[3][MAX_BOUNDARY_TYPES];
int boundaryNodePtr[3][MAX_BOUNDARY_TYPES];
int boundaryEdgePtr[3][MAX_BOUNDARY_TYPES];

int numNodes;
int isBoundaryNode[MAX_NODES];
int boundaryNode[MAX_NODES];
float bndNodeData[MAX_NODES];
//float  node_coordinates[MAX_NODES][3];
float velocity[MAX_NODES][3];
float forces[MAX_NODES][3];
float vel_delta[MAX_NODES][3];
float vel_backup[MAX_NODES][3];
float vel_boundary[MAX_NODES][3];
float tmpA[MAX_NODES][3];

int numEdges;
//read-only
int edge[MAX_EDGES][2];
//read-only
float edgeData[MAX_EDGES][5];
int isBoundaryEdge[MAX_EDGES];
int boundaryEdge[MAX_EDGES];
float bndEdgeData[MAX_EDGES];

int numFaces;
//read-only
int face[MAX_FACES][4];
//read-only
float faceData[MAX_FACES][3];
int boundaryFace[MAX_FACES];
int isBoundaryFace[MAX_FACES];

int proc_id, nprocs, rrkdebug;
int node_ptr[64], edge_ptr[64], face_ptr[64];
float shared_vel[64], shared_v_tot[64];
float shared_buf[64][MAX_NODES][3];
//coordinates are stored in buf in ReadMesh
float buf[MAX_NODES][3];
int local_numBoundaryEdges, local_numBoundaryNodes;

char start_pad[4096];
char pad1[4096];
char pad2[4096];
char pad3[4096];
char pad4[4096];
char pad5[4096];
char pad6[4096];
char pad7[4096];
char end_pad[4096];

int contribe[64][MAX_NODES];
int contribef[64][MAX_NODES];
int numce[64];
int numcef[64];

int icontribe[64][MAX_NODES];
int icontribef[64][MAX_NODES];
int inumce[64];
int inumcef[64];


#endif
