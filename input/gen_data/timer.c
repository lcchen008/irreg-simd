#include <stdio.h>
#include <sys/time.h>

struct timeval lt;	/* save previous time when lap_timer() called */
static int first = 1;

void set_timer()
{
   if (first) {
      gettimeofday(&lt, NULL);
      first = 0;
   }
}

void reset_timer()
{
   gettimeofday(&lt, NULL);
}

double lap_timer()
{
   struct timeval tt;
   double lap_time;

   gettimeofday(&tt, NULL);
   lap_time = (double)(tt.tv_sec - lt.tv_sec) + 
	      (double)(tt.tv_usec - lt.tv_usec)/1000000.0;
   lt = tt;
   return lap_time;	
}

double get_timer()
{
   struct timeval tt;
   double lap_time;

   if (first) { 
	set_timer(); 
	return 0.0;
   }
   gettimeofday(&tt, NULL);
   lap_time = (double)(tt.tv_sec - lt.tv_sec) + 
	      (double)(tt.tv_usec - lt.tv_usec)/1000000.0;
   return lap_time;	
}
/* fortran compatibility */

void set_timer_()
{
   set_timer();
}

void reset_timer_()
{
   reset_timer();
}

double lap_timer_()
{
   return lap_timer();
}

double get_timer_()
{
   return get_timer();
}
