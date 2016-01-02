//
//  vfloat.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vdouble_h
#define SSE_Template_vdouble_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vdouble_mask.h"
#include "vint.h"


class vdouble
{
public:
    __SSEd val;
    
public:
    //implict convert double to vdouble
    //default value are zeros
    vdouble()
    :val(_mm_set1_pd(0.0))
    {}
    
    //implict convert double to vdouble
    vdouble(double _val)
    :val(_mm_set1_pd(_val))
    {}
    
    //SIMD variable initialization
    vdouble(__SSEd _val)
    :val(_val)
    {}
    
    //implict convert vfloat to __SSE
    inline operator __SSEd()
    {
        return val;
    }
    
    //convert to MASK::vfloat type
    MASK::vdouble& mask()
    {
        return *(MASK::vdouble *)this;
    }
    
    /*************** Load and Store Operations**************/
    //load continuous memory space
    void load(void *src){
        val = _mm_load_pd(src);
    }
    
    //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
    void load(void *src, const vint &index, int scale){
        val = _mm_gather_pd(index.val, src, scale);
    }
    
    //store to continuous memory space
    void store(void *dst){
        return _mm_store_pd(dst, val);
    }
    
    //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
    void store(void *dst, const vint &index, int scale){
        return _mm_scatter_pd(dst, index.val, val, scale);
    }
    /*********************************************************/
    
    inline vdouble& operator = (const vdouble &b){
        val = b.val;
        return *this;
    }
    
    inline vdouble& operator = (double b){
        val = _mm_set1_pd(b);
        return *this;
    }
    
    inline vdouble& operator += (const vdouble &b){
        val = _mm_add_pd(val, b.val);
        return *this;
    }
    
    inline vdouble& operator -= (const vdouble &b){
        val = _mm_sub_pd(val, b.val);
        return *this;
    }
    
    inline vdouble& operator *= (const vdouble &b){
        val = _mm_mul_pd(val, b.val);
        return *this;
    }
    
    inline vdouble& operator /= (const vdouble &b){
        val = _mm_div_pd(val, b.val);
        return *this;
    }
    
    inline const vdouble sqrt(){
        vdouble tmp = _mm_sqrt_pd(val);
        return tmp;
    }
};

/******************************Float Operations******************************************/

inline const vdouble operator + (vdouble a, const vdouble &b)
{
    a += b;
    return a;
}

inline const vdouble operator - (vdouble a, const vdouble &b)
{
    a -= b;
    return a;
}

inline const vdouble operator * (vdouble a, const vdouble &b)
{
    a *= b;
    return a;
}

inline const vdouble operator / (vdouble a, const vdouble &b)
{
    a /= b;
    return a;
}

inline const vdouble sqrt(vdouble &a)
{
    return a.sqrt();
}

/**********************************************************/

/*********************Logical Operation***********************/

inline mask operator < (const vdouble &a, const vdouble &b)
{
    return _mm_cmplt_pd(a.val, b.val);
}

inline mask operator <= (const vdouble &a, const vdouble &b)
{
    return _mm_cmple_pd(a.val, b.val);
}

inline mask operator > (const vdouble &a, const vdouble &b)
{
    return _mm_cmpnle_pd(a.val, b.val);
}

inline mask operator >= (const vdouble &a, const vdouble &b)
{
    return _mm_cmpnlt_pd(a.val, b.val);
}

inline mask operator == (const vdouble &a, const vdouble &b)
{
    return _mm_cmpeq_pd(a.val, b.val);
}

inline mask operator != (const vdouble &a, const vdouble &b)
{
    return _mm_cmpneq_pd(a.val, b.val);
}

/*********************************************************/

/***************Set Operation**********************/

inline vdouble set(double val)
{
    return vdouble(val);
}

/**********************************************************/



#endif
