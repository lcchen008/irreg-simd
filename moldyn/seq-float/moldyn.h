#ifndef MOLDYN_H
#define MOLDYN_H

/************************************************************
!File     : moldyn.h
!Created  : Shamik Sharma, 
!Modified : Shamik Sharma,
!Modified : Hwansoo Han, 
!
!Description :  Header file for moldyn.h
*************************************************************/


#define SQRT(a)  sqrt(a)
#define POW(a,b) pow(a,b)
#define SQR(a)   ((a)*(a))
#define DRAND  drand_x


extern long myrandom();

/*********************
!======================  DATA-SETS  ======================================
#define  VERBOSE
**********************/
#define  XLARGE

# ifdef  SMALL0
#      define BOXSIZE                 2    /* creates 256 molecules */
#      define MAXINTERACT          2000    /* size of interaction array */
# elif defined(SMALL)
#      define BOXSIZE                 4    /* creates 256 molecules */
#      define MAXINTERACT         32000    /* size of interaction array */
# elif defined(MEDIUM)
#      define BOXSIZE                 8    /* create 2048 molecules */
#      define MAXINTERACT        320000 
# elif defined(LARGE) 
#      define BOXSIZE                16	   /* create 16384 molecules */
#      define MAXINTERACT       5000000 
# elif defined(XLARGE) 
#      define BOXSIZE                49	   /* create 131072 molecules */
#      define MAXINTERACT       31000000    /* needs 49 for auto.graph */
# elif defined(EXTREME) 
#      define BOXSIZE                64	   /* create 1048576 molecules */
#      define MAXINTERACT      11944000
# endif     

#define NTIMESTEP	   50

#define NUM_PARTICLES      (12*BOXSIZE*BOXSIZE*BOXSIZE)
#define DENSITY            0.83134         /* originally 0.83134 */
#define TEMPERATURE        0.722           /* originally 0.722 */
#define CUTOFF             3.5000
#define DEFAULT_TIMESTEP   0.064
#define SCALE_TIMESTEP     (4.0 )          /* originally 4 */
#define TOLERANCE          1.2  

#define DIMSIZE NUM_PARTICLES
#define MIN(a,b)     (((a)<(b))?(a):(b))       


/*
!======================  GLOBAL ARRAYS ======================================
!
! Note : inter is usually the biggest array. If BOXSIZE = 13, it will
!        cause 1 million interactions to be generated. This will need
!        a minimum of 80 MBytes to hold 'inter'. The other
!        arrays will need about a sum of around 800 KBytes. Note
!        that MAXINTERACT may be defined to a more safe value causing
!        extra memory to be allocated. (~ 130 MBytes !)
!============================================================================
*/

#ifdef NO_NEED
EXTERN
double  vhx[NUM_PARTICLES],   /* partial x,y,z velocity of molecule */
        vhy[NUM_PARTICLES],   
        vhz[NUM_PARTICLES];   

EXTERN
double  x  [NUM_PARTICLES],   /* x,y,z coordinates of each molecule */
        y  [NUM_PARTICLES],  
        z  [NUM_PARTICLES]; 

EXTERN
double  fx [NUM_PARTICLES],   /* partial forces on each molecule    */
        fy [NUM_PARTICLES], 
        fz [NUM_PARTICLES];


/* (inter1, inter2) stores pairs of interacting molecules */ 
EXTERN int     *inter1, *inter2;

/*
!======================  GLOBAL VARIABLES ===================================
*/

EXTERN
double   side,                  /*  length of side of box                 */ 
         sideHalf,              /*  1/2 of side                           */
         cutoffRadius,          /*  cuttoff distance for interactions     */
         perturb,               /*  perturbs initial coordinates          */ 
         timeStep,              /*  length of each timestep   */
         timeStepSq,            /*  square of timestep        */
         timeStepSqHalf,        /*  1/2 of square of timestep */
         vaver,                 /*                            */
         epot,                  /*  The potential energy      */
         vir;                   /*  The virial  energy        */

EXTERN
int      numMoles,              /*  number of molecules                   */
         neighUpdate,           /*  timesteps between interaction updates */
         ninter,                /*  number of interacting molecules pairs */
         numProcs,              /*  number of processors to use           */
         numTimeSteps;          /*  number of time-steps to simulate for  */

#endif /* NO_NEED */

#ifdef NO_NEED
#define  MAX_TIMERS 20
#define  TT_NET     0
#define  TT_UPCOOR  1
#define  TT_BNEIGH  2
#define  TT_BNEIGH1 3
#define  TT_BNEIGH2 4
#define  TT_INSP    5
#define  TT_COMPF   6
#define  TT_COMPF1  7
#define  TT_COMPF2  8
#define  TT_COMPF3  9
#define  TT_UPVEL   10


EXTERN
double  time_tot[MAX_TIMERS], 
         time_st[MAX_TIMERS], 
         time_avg[MAX_TIMERS], 
         time_max[MAX_TIMERS], 
         time_min[MAX_TIMERS];

#define MAX_STATS 20
#define TT_ATOM  0
#define TT_ELEM  1
#define TT_ITER  8

EXTERN  
int     stats    [MAX_STATS], 
        stats_max[MAX_STATS],
        stats_min[MAX_STATS], 
        stats_avg[MAX_STATS];
#endif /* NO_NEED */

void  UpdateCoordinates();
void  BuildNeigh();
void  UpdateVelocities();
void  ComputeForces();
void  InitSettings();
void  InitCoordinates();
void  InitVelocities();
void  InitForces();
void  ComputeKE(double *ekin);
void  ComputeAvgVel(double *vel, double *count);
void  PrintResults(int move, float ekin, float vel, float count);

#ifdef RCB_VAR
void  InitRcbVar();
#endif

#endif /* MOLDYN_H */
