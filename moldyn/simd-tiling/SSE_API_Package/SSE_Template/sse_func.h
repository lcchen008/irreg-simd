#ifndef _SSE_FUNC
#define _SSE_FUNC

#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>

using namespace std;

//The width of one sse lane
//for integer computation
#define WIDTH 32

#if WIDTH == 64
    typedef int64_t d_type;
    #define ALL1 0xffffffffffffffff
#elif WIDTH == 32
    typedef int32_t d_type;
    #define ALL1 0xffffffff
#elif WIDTH == 16
    typedef int16_t d_type;
    #define ALL1 0xffff
#elif WIDTH == 8
    typedef int8_t d_type;
    #define ALL1 0xff
#endif


/****************SSE Instructions****************/
#define TOKENPASTE2(x, y) x ## y
#define TOKENPASTE(x, y) TOKENPASTE2(x, y) 

/********************************MIC Platform*****************************/
/*******************************512 bits vector**************************/
/***********Type*************/
#ifdef MIC
#define __SSE  __m512  //float
#define __SSEd __m512d //double
#define __SSEi __m512i //int
#define mask __mmask16 //mask

/*******************************Double Instructions**********************/
/*******Arithmetic Operations***/
#define _mm_add_pd  _mm512_add_pd
#define _mm_sub_pd  _mm512_sub_pd
#define _mm_mul_pd  _mm512_mul_pd
#define _mm_sqrt_pd _mm512_sqrt_pd
#define _mm_div_pd  _mm512_div_pd

#define _mm_mask_add_pd  _mm512_mask_add_pd
#define _mm_mask_sub_pd  _mm512_mask_sub_pd
#define _mm_mask_mul_pd  _mm512_mask_mul_pd
#define _mm_mask_sqrt_pd  _mm512_mask_sqrt_pd
#define _mm_mask_div_pd  _mm512_mask_div_pd

/*********SET/LOAD/STORE/MOV Operations*******/
#define _mm_set1_pd _mm512_set1_pd
#define _mm_set_pd  _mm512_set_pd
#define _mm_store_pd _mm512_store_pd
#define _mm_load_pd _mm512_load_pd
//gather and scatter
#define _mm_gather_pd _mm512_i32logather_pd
#define _mm_scatter_pd _mm512_i32loscatter_pd
//mask operations
#define _mm_mask_mov_pd _mm512_mask_mov_pd
#define _mm_mask_load_pd _mm512_mask_load_pd
#define _mm_mask_store_pd _mm512_mask_store_pd
#define _mm_mask_gather_pd _mm512_mask_i32logather_pd
#define _mm_mask_scatter_pd _mm512_mask_i32loscatter_pd

/************Logic Operations*************/
#define _mm_cmpeq_pd _mm512_cmpeq_pd_mask
#define _mm_cmplt_pd _mm512_cmplt_pd_mask
#define _mm_cmple_pd _mm512_cmple_pd_mask
#define _mm_cmpneq_pd _mm512_cmpneq_pd_mask
#define _mm_cmpnlt_pd _mm512_cmpnlt_pd_mask
#define _mm_cmpnle_pd _mm512_cmpnle_pd_mask
#define _mm_cmpord_pd _mm512_cmpord_pd_mask
#define _mm_cmpunord_pd _mm512_cmpunord_pd_mask

/**************************************************************************/

/*******************************Float Instructions**********************/
/*******Arithmetic Operations***/
#define _mm_add_ps  _mm512_add_ps
#define _mm_sub_ps  _mm512_sub_ps
#define _mm_mul_ps  _mm512_mul_ps
#define _mm_sqrt_ps _mm512_rsqrt23_ps
#define _mm_div_ps  _mm512_div_ps

#define _mm_mask_add_ps  _mm512_mask_add_ps
#define _mm_mask_sub_ps  _mm512_mask_sub_ps
#define _mm_mask_mul_ps  _mm512_mask_mul_ps
#define _mm_mask_sqrt_ps  _mm512_mask_rsqrt23_ps
#define _mm_mask_div_ps  _mm512_mask_div_ps

/*********SET/LOAD/STORE/MOV Operations*******/
#define _mm_set1_ps _mm512_set1_ps
#define _mm_set_ps  _mm512_set_ps
#define _mm_store_ps _mm512_store_ps
#define _mm_load_ps _mm512_load_ps
//gather and scatter
#define _mm_gather_ps _mm512_i32gather_ps
#define _mm_scatter_ps _mm512_i32scatter_ps
//mask operations
#define _mm_mask_mov_ps _mm512_mask_mov_ps
#define _mm_mask_load_ps _mm512_mask_load_ps
#define _mm_mask_store_ps _mm512_mask_store_ps
#define _mm_mask_gather_ps _mm512_mask_i32gather_ps
#define _mm_mask_scatter_ps _mm512_mask_i32scatter_ps

/************Logic Operations*************/
#define _mm_cmpeq_ps _mm512_cmpeq_ps_mask
#define _mm_cmplt_ps _mm512_cmplt_ps_mask
#define _mm_cmple_ps _mm512_cmple_ps_mask
#define _mm_cmpneq_ps _mm512_cmpneq_ps_mask
#define _mm_cmpnlt_ps _mm512_cmpnlt_ps_mask
#define _mm_cmpnle_ps _mm512_cmpnle_ps_mask
#define _mm_cmpord_ps _mm512_cmpord_ps_mask
#define _mm_cmpunord_ps _mm512_cmpunord_ps_mask


/***************************Integer Instructions*******************/
/*****************Arithmetic Operations**********/
#define _mm_add_i TOKENPASTE(_mm512_add_epi,WIDTH)
#define _mm_adds_i TOKENPASTE(_mm512_adds_epi,WIDTH)
#define _mm_sub_i TOKENPASTE(_mm512_sub_epi,WIDTH)
#define _mm_subs_i TOKENPASTE(_mm512_subs_epi,WIDTH)
#define _mm_mul_i TOKENPASTE(_mm512_mullo_epi,WIDTH)
#define _mm_div_i TOKENPASTE(_mm512_div_epi, WIDTH)

#define _mm_mask_add_i  TOKENPASTE(_mm512_mask_add_epi,WIDTH)
#define _mm_mask_sub_i  TOKENPASTE(_mm512_mask_sub_epi,WIDTH)
#define _mm_mask_mul_i  TOKENPASTE(_mm512_mask_mullo_epi,WIDTH)
#define _mm_mask_div_i  TOKENPASTE(_mm512_mask_div_epi, WIDTH)

/*****************SET/LOAD/STORE/MOV Operations*********/
//_mm_set1_epi(64,32,16,8)
#define _mm_set1_i TOKENPASTE(_mm512_set1_epi,WIDTH)
#define _mm_set_i TOKENPASTE(_mm512_set_epi,WIDTH)
#define _mm_store_i TOKENPASTE(_mm512_store_epi,WIDTH)
#define _mm_load_i TOKENPASTE(_mm512_load_epi,WIDTH)
//gather and scatter
#define _mm_gather_i _mm512_i32gather_epi32
#define _mm_scatter_i _mm512_i32scatter_epi32
//mov operation with mask
#define _mm_mask_mov_i TOKENPASTE(_mm512_mask_mov_epi,WIDTH)
#define _mm_mask_load_i TOKENPASTE(_mm512_mask_load_epi,WIDTH)
#define _mm_mask_store_i TOKENPASTE(_mm512_mask_store_epi,WIDTH)
#define _mm_mask_gather_i TOKENPASTE(_mm512_mask_i32gather_epi, WIDTH)
#define _mm_mask_scatter_i TOKENPASTE(_mm512_mask_i32scatter_epi, WIDTH)

/****************Logic Operations******************/
#define _mm_cmpeq_i TOKENPASTE(_mm512_cmpeq_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmplt_i TOKENPASTE(_mm512_cmplt_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmple_i TOKENPASTE(_mm512_cmple_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmpneq_i TOKENPASTE(_mm512_cmpeq_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmpnlt_i TOKENPASTE(_mm512_cmpge_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmpnle_i TOKENPASTE(_mm512_cmpgt_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmpord_i TOKENPASTE(_mm512_cmpord_epi, TOKENPASTE(WIDTH, _mask))
#define _mm_cmpunord_i TOKENPASTE(_mm512_cmpunord_epi, TOKENPASTE(WIDTH, _mask))


/*********************************************************************************/

#else

/*************************************SSE Platform************************************/
/*************************************128 bits vector********************************/
#define __SSE  __m128
#define __SSEi __m128i

/*************************************Integer Instructions****************************/
/************Arithmetic Intructions***********/
#define _mm_add_i TOKENPASTE(_mm_add_epi,WIDTH)
#define _mm_adds_i TOKENPASTE(_mm_adds_epi,WIDTH)
#define _mm_sub_i TOKENPASTE(_mm_sub_epi,WIDTH)
#define _mm_subs_i TOKENPASTE(_mm_subs_epi,WIDTH)
#define _mm_mul_i TOKENPASTE(_mm_mul_epi,WIDTH)

/*************SET/Load/Store Intructions**********/
#define _mm_set1_i TOKENPASTE(_mm_set1_epi,WIDTH)
#define _mm_set_i TOKENPASTE(_mm_set_epi,WIDTH)
#define _mm_setr_i TOKENPASTE(_mm_setr_epi,WIDTH)
#define _mm_load_i TOKENPASTE(_mm_load_epi,WIDTH)
#define _mm_store_i TOKENPASTE(_mm_store_epi,WIDTH)

/************Logic Instruction**************/
#define _mm_cmpeq_i TOKENPASTE(_mm_cmpeq_epi,WIDTH)
#define _mm_cmpgt_i TOKENPASTE(_mm_cmpgt_epi,WIDTH)
#define _mm_cmplt_i TOKENPASTE(_mm_cmplt_epi,WIDTH)
/*********************************************************************************/
#endif


template<typename SSE_T, typename T>
class SSE_func
{
public:
    static void print(const SSE_T &n)
    {
        string sse_name = typeid(SSE_T).name();
        //cout<<sse_name<<endl;
        if(sse_name == typeid(__m128i).name())
        {
            T *tmp = (T *)&n;
            cout<<"(";
            for(int i = 0; i < 128/(sizeof(T)*8); i++){
                cout<<tmp[i]<<", ";
            }
            cout<<")"<<endl;
        }
        else if(sse_name == typeid(__m128).name())
        {
            T *tmp = (T*)&n;
            cout<<"(";
            for(int i = 0; i < 128/(sizeof(T)*8); i++){
                cout<<fixed<<setprecision(3)
                <<tmp[i]<<", ";
            }
            cout<<")"<<endl;
        }
        else if(sse_name == typeid(__m512i).name())
        {
            T *tmp = (T*)&n;
            cout<<"(";
            for(int i = 0; i < 512/(sizeof(T)*8); i++){
                cout<<tmp[i]<<", ";
            }
            cout<<")"<<endl;
        }
        else if(sse_name == typeid(__m512).name())
        {
            T *tmp = (T*)&n;
            cout<<"(";
            for(int i = 0; i < 512/(sizeof(T)*8); i++){
                cout<<fixed<<setprecision(3)
                <<tmp[i]<<", ";
            }
            cout<<")"<<endl;
        }
    }
    
};

#endif
