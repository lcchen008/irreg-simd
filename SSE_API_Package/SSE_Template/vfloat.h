//
//  vfloat.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vfloat_h
#define SSE_Template_vfloat_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vfloat_mask.h"
#include "vint.h"


class vfloat
{
public:
    __SSE val;
    
public:
    //implict convert float to vfloat
    //default value are zeros
    vfloat()
    :val(_mm_set1_ps(0.0))
    {}
    
    //implict convert float to vfloat
    vfloat(float _val)
    :val(_mm_set1_ps(_val))
    {}
    
    //SIMD variable initialization
    vfloat(__SSE _val)
    :val(_val)
    {}
    
    //implict convert vfloat to __SSE
    inline operator __SSE()
    {
        return val;
    }
    
    //convert to MASK::vfloat type
    MASK::vfloat& mask()
    {
        return *(MASK::vfloat *)this;
    }
    
    /*************** Load and Store Operations**************/
    //load continuous memory space
    void load(void *src){
        val = _mm_load_ps(src);
    }
    
    //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
    void load(void *src, const vint &index, const int scale){
        val = _mm_gather_ps(index.val, src, 4);
    }
    
    //store to continuous memory space
    void store(void *dst){
        return _mm_store_ps(dst, val);
    }
    
    //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
    void store(void *dst, const vint &index, int scale){
        return _mm_scatter_ps(dst, index.val, val, scale);
    }
    /*********************************************************/
    
    inline vfloat& operator = (const vfloat &b){
        val = b.val;
        return *this;
    }
    
    inline vfloat& operator = (float b){
        val = _mm_set1_ps(b);
        return *this;
    }
    
    inline vfloat& operator += (const vfloat &b){
        val = _mm_add_ps(val, b.val);
        return *this;
    }
    
    inline vfloat& operator -= (const vfloat &b){
        val = _mm_sub_ps(val, b.val);
        return *this;
    }
    
    inline vfloat& operator *= (const vfloat &b){
        val = _mm_mul_ps(val, b.val);
        return *this;
    }
    
    inline vfloat& operator /= (const vfloat &b){
        val = _mm_div_ps(val, b.val);
        return *this;
    }
    
    inline const vfloat sqrt(){
        vfloat tmp = _mm_mul_ps(val, _mm512_rsqrt23_ps(val));
        return tmp;
    }
};

/******************************Float Operations******************************************/

inline const vfloat operator + (vfloat a, const vfloat &b)
{
    a += b;
    return a;
}

inline const vfloat operator - (vfloat a, const vfloat &b)
{
    a -= b;
    return a;
}

inline const vfloat operator * (vfloat a, const vfloat &b)
{
    a *= b;
    return a;
}

inline const vfloat operator / (vfloat a, const vfloat &b)
{
    a /= b;
    return a;
}

inline const vfloat sqrt(vfloat &a)
{
    return a.sqrt();
}

/**********************************************************/

/*********************Logical Operation***********************/

inline mask operator < (const vfloat &a, const vfloat &b)
{
    return _mm_cmplt_ps(a.val, b.val);
}

inline mask operator <= (const vfloat &a, const vfloat &b)
{
    return _mm_cmple_ps(a.val, b.val);
}

inline mask operator > (const vfloat &a, const vfloat &b)
{
    return _mm_cmpnle_ps(a.val, b.val);
}

inline mask operator >= (const vfloat &a, const vfloat &b)
{
    return _mm_cmpnlt_ps(a.val, b.val);
}

inline mask operator == (const vfloat &a, const vfloat &b)
{
    return _mm_cmpeq_ps(a.val, b.val);
}

inline mask operator != (const vfloat &a, const vfloat &b)
{
    return _mm_cmpneq_ps(a.val, b.val);
}

/*********************************************************/

/***************Set Operation**********************/

inline vfloat set(float val)
{
    return vfloat(val);
}

/**********************************************************/



#endif
