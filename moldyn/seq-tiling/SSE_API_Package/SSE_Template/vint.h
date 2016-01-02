//
//  vint.h
//  SSE Template
//
//  Created by Xin on 10/1/13.
//
//

#ifndef SSE_Template_vint_h
#define SSE_Template_vint_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vint_mask.h"

class vint
{
public:
    __SSEi val;
    
public:
    //implict convert int to vint
    //default value are zeros
    vint()
    :val(_mm_set1_i(0))
    {}
    
    //implict convert SSEi to vint
    vint(__SSEi _val)
    :val(_val)
    {}

	vint(const vint& v) : val(v.val){
	}
	
    vint(int e0, int e1, int e2, int e3, int e4, int e5, int e6, int e7, int e8, int e9, int e10, int e11, int e12, int e13, int e14, int e15) {
	val = _mm512_set_epi32(e15, e14, e13, e12, e11, e10, e9, e8, e7, e6, e5, e4, e3, e2, e1, e0);
    }
    
    //implict convert vint to __SSEi
    operator __SSEi()
    {
        return val;
    }
    
    vint(int _val)
    :val(_mm_set1_i(_val))
    {}
    
    //convert to MASK::vint
    MASK::vint& mask()
    {
        return *(MASK::vint*)this;
    }
    
    /*************** Load and Store Operations**************/
    //load continuous memory space
    void load(void *src){
        val = _mm_load_i(src);
    }
    
    //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
    void load(void *src, const vint &index, int scale){
        val = _mm_gather_i(index.val, src, scale);
    }
    
    //store to continuous memory space
    void store(void *dst){
        return _mm_store_i(dst, val);
    }
    
    //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
    void store(void *dst, const vint &index, int scale){
        return _mm_scatter_i(dst, index.val, val, scale);
    }
    /*******************************************************/
    
    inline vint& operator = (const vint &b){
        val = b.val;
        return *this;
    }
    
    inline vint& operator = (int b){
        val = _mm_set1_i(b);
        return *this;
    }
    
    inline vint& operator += (const vint &b){
        val = _mm_add_i(val, b.val);
        return *this;
    }
    
    inline vint& operator -= (const vint &b){
        val = _mm_sub_i(val, b.val);
        return *this;
    }
    
    inline vint& operator *= (const vint &b){
        val = _mm_mul_i(val, b.val);
        return *this;
    }
    
    inline vint& operator /= (const vint &b){
        val = _mm_div_i(val, b.val);
        return *this;
    }
    
    inline const vint sqrt(){
        //convert from __SSEi to __SSE
        __SSE tmp = _mm512_castsi512_ps(val);
        //Compute sqrt
        tmp = _mm_mul_ps(tmp, _mm512_rsqrt23_ps(tmp));
        //convert from __SSE to __SSEi
        __SSEi tmp2 = _mm512_castps_si512(tmp);
        
        return vint(tmp2);
    }
    
    //gather addr+index[i]*scale, scale has to be 0, 2, 4, 8...
    void gather(const vint &index, void const* addr, int scale)
    {
        val = _mm512_i32gather_epi32(index.val, addr, scale);
    }
    
    //scatter addr+index[i]*scale, scale has to be 0, 2, 4, 8...
    void scatter(const vint &index, void *addr, int scale)
    {
        _mm512_i32scatter_epi32(addr, val, index.val, scale);
    }
};

/*****************Integer Arithmetic Operation******************/
inline const vint operator + (vint a, const vint &b)
{
    a += b;
    return a;
}

inline const vint operator - (vint a, const vint &b)
{
    a -= b;
    return a;
}

inline const vint operator * (vint a, const vint &b)
{
    a *= b;
    return a;
}

inline const vint operator / (vint a, const vint &b){
    a /= b;
    return a;
}

/*************************************************************/

/*********************Logical Operation***********************/

inline mask operator < (const vint &a, const vint &b)
{
    return _mm_cmplt_i(a.val, b.val);
}

inline mask operator <= (const vint &a, const vint &b)
{
    return _mm_cmple_i(a.val, b.val);
}

inline mask operator > (const vint &a, const vint &b)
{
    return _mm_cmpnle_i(a.val, b.val);
}

inline mask operator >= (const vint &a, const vint &b)
{
    return _mm_cmpnlt_i(a.val, b.val);
}

inline mask operator == (const vint &a, const vint &b)
{
    return _mm_cmpeq_i(a.val, b.val);
}

inline mask operator != (const vint &a, const vint &b)
{
    return _mm_cmpneq_i(a.val, b.val);
}

/*********************************************************/

/***************Set/Load/Store Operation**********************/

inline const vint set(int val)
{
    return vint(val);
}
/**********************************************************/

#endif
