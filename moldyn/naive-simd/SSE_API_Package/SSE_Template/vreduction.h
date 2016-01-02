//
//  vreduction.h
//  SSE Template
//
//  Created by Xin Huo on 10/1/13.
//
//

#ifndef SSE_Template_sreduction_h
#define SSE_Template_sreduction_h
#include <smmintrin.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include "sse_func.h"

/*************************************Reduction API***********************************/
template<typename T>
class Add{
public:
    const T operator ()(T &lsh, const T &rsh)
    {
        return (lsh+=rsh);
    }
};

template<typename T>
class Sub{
public:
    const T operator ()(T &lsh, const T &rsh)
    {
        return (lsh-=rsh);
    }
};

template<typename T>
class Mul{
public:
    const T operator ()(T &lsh, const T &rsh)
    {
        return (lsh*=rsh);
    }
};

template<typename T>
class Div{
public:
    const T operator ()(T &lsh, const T &rsh)
    {
        return (lsh/=rsh);
    }
};

template<int vec_width=16, class reducComp = Add<float> >
inline void reduction(float *update, int scale, int offset, __SSEi update_index, __SSE value)
{
    reducComp op;
    for(int ri = 0; ri < vec_width; ++ri)
    {
        op(update[((int*)&update_index)[ri]*scale+offset], ((float*)&value)[ri]);
    }
}

template<int vec_width=16, class reducComp = Add<float> >
inline void reduction(int *update, int scale, int offset, __SSEi update_index, __SSEi value)
{
    reducComp op;
    for(int ri = 0; ri < vec_width; ++ri)
    {
        op(update[((int*)&update_index)[ri]*scale+offset], ((int*)&value)[ri]);
    }
}

template<int vec_width=16, class reducComp = Add<float> >
inline void reduction(float *update, int scale, int offset, __SSEi update_index, float value)
{
    reducComp op;
    for(int ri = 0; ri < vec_width; ++ri)
    {
        reducComp()(update[((int*)&update_index)[ri]*scale+offset], value);
    }
}

template<int vec_width=16, class reducComp = Add<int> >
inline void reduction(int *update, int scale, int offset, __SSEi update_index, mask value)
{
    reducComp op;
    for(int ri = 0; ri < vec_width; ++ri)
    {
        op(update[((int*)&update_index)[ri]*scale+offset], ((short)value>>ri)&1);
    }
}

/********************************************************************************************/

#endif
