//
//  vint.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_vint_mask_h
#define SSE_Template_vint_mask_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <cstdio>
#include "sse_func.h"
#include "vmask.h"
#include "vint.h"
#include "vfloat.h"


namespace MASK {
    
    class vint
    {
    public:
        __SSEi val;
        
    public:
        vint()
        :val(_mm_set1_i(0))
        {}
        
        //support implict convert int to vint
        vint(__SSEi _val)
        :val(_val)
        {}
        
        vint(int _val)
        :val(_mm_set1_i(_val))
        {}
        
        //support implict convert vint to __SSEi
        operator __SSEi()
        {
            return val;
        }
  void print() {
	for(int i=0;i<16;i++) {
	  cout << *((int*)&val+i) << " ";
	}
	cout << endl;
    }
       
        /*************** Load and Store Operations**************/
        //load continuous memory space
        void load(void *src){
            val = _mm_mask_load_i(Mask::oldi, Mask::m, src);
        }
        
        //load non-continuous memory space, src+index[i]*scale, scale has to be 0, 2, 4, 8...
        void load(void *src, const vint &index, int scale){
            val = _mm_mask_gather_i(Mask::oldi, Mask::m, index.val, src, scale);
        }
        
        //store to continuous memory space
        void store(void *dst){
            return _mm_mask_store_i(dst, Mask::m, val);
        }
        
        //store to non-continuous memory space, dst+index[i]*scale, scale has to be 0, 2, 4, 8...
        void store(void *dst, const vint &index){
            return _mm_mask_scatter_i(dst, Mask::m, index.val, val, 4);
        }
        
        inline vint& operator = (const vint &b){
            val = _mm_mask_mov_i(Mask::oldi, Mask::m, b.val);
            return *this;
        }
        
        inline vint& operator = (int b){
            val = _mm_mask_mov_i(Mask::oldi, Mask::m, _mm_set1_i(b));
            return *this;
        }
        
        inline vint& operator += (const vint &b){
            val = _mm_mask_add_i(Mask::oldi, Mask::m, val, b.val);
            return *this;
        }

	inline vint& operator &= (const vint &b){
		val = _mm512_mask_and_epi32(Mask::oldi, Mask::m, val, b.val);
		return *this;
	}

	inline vint& operator -= (const vint &b){
		val = _mm_mask_sub_i(Mask::oldi, Mask::m, val, b.val);
		return *this;
	}

        inline vint& operator *= (const vint &b){
            val = _mm_mask_mul_i(Mask::oldi, Mask::m, val, b.val);
            return *this;
        }
        
        inline vint& operator /= (const vint &b){
            val = _mm_mask_div_i(Mask::oldi, Mask::m, val, b.val);
            return *this;
        }
        
        inline const vint sqrt(){
            //convert from __SSEi to __SSE
            __SSE tmp = _mm512_castsi512_ps(val);
            //Compute sqrt
            tmp = _mm_mul_ps(tmp, _mm512_rsqrt23_ps(tmp));
            //convert from __SSE to __SSEi
            //__SSEi tmp2 = _mm512_castps_si512(tmp);
            __SSEi tmp2 = _mm512_castps_si512(tmp);
            __SSEi tmp3 = _mm_mask_mov_i(Mask::oldi, Mask::m, tmp2);
            
            return vint(tmp3);
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
        return vint(_mm_mask_mov_i(Mask::oldi, Mask::m, _mm_set1_i(val)));
    }
    
    /**********************************************************/
}


#endif
