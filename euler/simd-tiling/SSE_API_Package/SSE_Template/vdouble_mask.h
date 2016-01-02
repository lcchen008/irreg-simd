//
//  vfloat.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vdouble_mask_h
#define SSE_Template_vdouble_mask_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vfloat.h"
#include "vint.h"


namespace MASK {
    
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
        
        /*************** Load and Store Operations**************/
        //load continuous memory space
        void load(void *src){
            val = _mm_mask_load_pd(Mask::oldd, Mask::m, src);
        }
        
        //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
        void load(void *src, const vint &index, int scale){
            val = _mm_mask_gather_pd(Mask::oldd, Mask::m, index.val, src, scale);
        }
        
        //store to continuous memory space
        void store(void *dst){
            return _mm_mask_store_pd(dst, Mask::m, val);
        }
        
        //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
        void store(void *dst, const vint &index, int scale){
            return _mm_mask_scatter_pd(dst, Mask::m, index.val, val, scale);
        }
        
        /**************************************************/
        
        inline vdouble& operator = (const vdouble &b){
            val = _mm_mask_mov_pd(Mask::oldd, Mask::m, b.val);
            return *this;
        }
        
        inline vdouble& operator = (double b){
            val = _mm_mask_mov_pd(Mask::oldd, Mask::m, _mm_set1_pd(b));
            return *this;
        }
        
        inline vdouble& operator += (const vdouble &b){
            val = _mm_mask_add_pd(Mask::oldd, Mask::m, val, b.val);
            return *this;
        }
        
        inline vdouble& operator -= (const vdouble &b){
            val = _mm_mask_add_pd(Mask::oldd, Mask::m, val, b.val);
            return *this;
        }
        
        inline vdouble& operator *= (const vdouble &b){
            val = _mm_mask_mul_pd(Mask::oldd, Mask::m, val, b.val);
            return *this;
        }
        
        inline vdouble& operator /= (const vdouble &b){
            val = _mm_mask_div_pd(Mask::oldd, Mask::m, val, b.val);
            return *this;
        }
        
        inline const vdouble sqrt(){
            vdouble tmp = _mm_mask_sqrt_pd(Mask::oldd, Mask::m, val);
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
    
    /***************Set/Load/Store Operation**********************/
    
    inline const vdouble set(double val)
    {
        return vdouble(_mm_mask_mov_pd(Mask::oldd, Mask::m, _mm_set1_pd(val)));
    }
    
    /**********************************************************/
}

#endif
