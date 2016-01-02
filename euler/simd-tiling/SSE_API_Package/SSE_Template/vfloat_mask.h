//
//  vfloat.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vfloat_mask_h
#define SSE_Template_vfloat_mask_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vfloat.h"
#include "vint.h"


namespace MASK {
    
    class vfloat
    {
    public:
        __SSE val;
        
    public:
        //support implict convert float to vfloat
        vfloat()
        :val(_mm_set1_ps(0.0))
        {}
        
        vfloat(__SSE _val)
        :val(_val)
        {}
        
        vfloat(float _val)
        :val(_mm_set1_ps(_val))
        {}
        
        //support implict convert vfloat to __SSE
        inline operator __SSE()
        {
            return val;
        }
        
        /*************** Load and Store Operations**************/
        //load continuous memory space
        void load(void *src){
            val = _mm_mask_load_ps(Mask::old, Mask::m, src);
        }
        
        //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
        void load(void *src, const vint &index, int scale){
            val = _mm_mask_gather_ps(Mask::old, Mask::m, index.val, src, scale);
        }
        
        //store to continuous memory space
        void store(void *dst){
            return _mm_mask_store_ps(dst, Mask::m, val);
        }
        
        //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
        void store(void *dst, const ::vint &index, int scale){
            return _mm_mask_scatter_ps(dst, Mask::m, index.val, val, scale);
        }
        
        inline vfloat& operator = (const vfloat &b){
            val = _mm_mask_mov_ps(Mask::old, Mask::m, b.val);
            return *this;
        }
        
        inline vfloat& operator = (float b){
            val = _mm_mask_mov_ps(Mask::old, Mask::m, _mm_set1_ps(b));
            return *this;
        }
        
        inline vfloat& operator += (const vfloat &b){
            val = _mm_mask_add_ps(Mask::old, Mask::m, val, b.val);
            return *this;
        }
        
        inline vfloat& operator -= (const vfloat &b){
            val = _mm_mask_add_ps(Mask::old, Mask::m, val, b.val);
            return *this;
        }
        
        inline vfloat& operator *= (const vfloat &b){
            val = _mm_mask_mul_ps(Mask::old, Mask::m, val, b.val);
            return *this;
        }
        
        inline vfloat& operator /= (const vfloat &b){
            val = _mm_mask_div_ps(Mask::old, Mask::m, val, b.val);
            return *this;
        }
        
        inline const vfloat sqrt(){
            vfloat tmp = _mm_mask_mul_ps(Mask::old, Mask::m, val, _mm512_rsqrt23_ps(val));
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
    
    /***************Set/Load/Store Operation**********************/
    
    inline const vfloat set(float val)
    {
        return vfloat(_mm_mask_mov_ps(Mask::old, Mask::m, _mm_set1_ps(val)));
    }
    
    /**********************************************************/
}

#endif
