//
//  vmask.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vmask_h
#define SSE_Template_vmask_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include "sse_func.h"

class Mask
{
public:
    static __thread mask m;
    static __thread __SSE old;
    static __thread __SSEi oldi;
    static __thread __SSEd oldd;
    
public:
    static void clear();
    static void set_mask(const mask &_mask, const __SSE &_old);
    static void set_mask(const mask &_mask, const __SSEi &_oldi);
    static void set_mask(const mask &_mask, const __SSEd &_oldd);
    static void print_mask();
};

__thread mask Mask::m;
__thread __SSE Mask::old;
__thread __SSEi Mask::oldi;
__thread __SSEd Mask::oldd;

void Mask::clear()
{
    old = _mm_set1_ps(0.0);
    oldi = _mm_set1_i(0);
    oldd = _mm_set1_pd(0.0);
    m = _mm512_int2mask(~0);
}

void Mask::set_mask(const mask &_mask, const __SSE &_old)
{
    m = _mask;
    old = _old;
}

void Mask::set_mask(const mask &_mask, const __SSEi &_oldi){
    m = _mask;
    oldi = _oldi;
}

void Mask::set_mask(const mask &_mask, const __SSEd &_oldd){
    m = _mask;
    oldd = _oldd;
}

void Mask::print_mask()
{
    for(int i = 0; i < 16; ++i){
        printf("%d ", (m>>i)&1);
    }
    printf("\n");
}

#endif
