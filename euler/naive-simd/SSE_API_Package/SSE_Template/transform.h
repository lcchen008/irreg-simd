//
//  transform.h
//  SSE Template
//
//  Created by MARS on 9/9/13.
//
//
#ifndef SSE_Template_transform_h
#define SSE_Template_transform_h

#include <iostream>
#include "omp.h"

using namespace std;

/**************Transform between Structure of Array and Array of structure***********/
template<typename T>
void transformTo(T *input, T *output, int nargc, int n)
{
    //Alligned to 64 bytes
    //T *output = (T*)_mm_malloc(n*sizeof(T), 64);
    omp_set_num_threads(61);
    for(int i = 0; i < nargc; ++i){
        #pragma omp parallel for schedule(static)
        for(int j = 0; j < n; j++){
            output[i*n+j] = input[j*nargc+i];
        }
    }
}

template<typename T>
void transformBack(T *input, T *output, int nargc, int n)
{
    omp_set_num_threads(61);
    for(int i = 0; i < nargc; ++i){
        #pragma omp parallel for schedule(static)
        for(int j = 0; j < n; j++){
            output[j*nargc+i] = input[i*n+j];
        }
    }
}

/***********************************************************************************/


/*************************Transform for stencil 2D transform********************/
//2D Transform
template<typename T, int vec_width>
void transformTo(T *input, T *&output, int row, int col)
{
    const int T_row = row/vec_width+2;
    const int T_col = row*vec_width;
    //output = (T*)_mm_malloc(T_row*T_col*sizeof(T), 64);
    
    T (*p_input)[col] = (T (*)[col])input;
    T (*p_output)[T_col] = (T (*)[T_col])output;
    
    omp_set_num_threads(61);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < T_col; i += vec_width){
        for(int j = 0; j < T_row; ++j){
            for(int t = 0; t < vec_width; ++t){
                int index = j+(col/vec_width)*t;
                if(index == 0 || index == col+1){
                    p_output[j][i+t] = 0;
                }
                else{
                    p_output[j][i+t] = p_input[i/vec_width][index-1];
                }
            }
        }
    }
}

template<typename T, int vec_width>
void transformBack(T *input, T *output, int row, int col)
{
    const int T_row = row/vec_width+2;
    const int T_col = row*vec_width;
    T (*p_input)[T_col] = (T (*)[T_col])input;
    T (*p_output)[col] = (T (*)[col])output;
    
    omp_set_num_threads(61);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < T_col; i += vec_width){
        for(int j = 1; j < T_row-1; ++j){
            for(int t = 0; t < vec_width; ++t){
                int index = j+(col/vec_width)*t;
                if(index > 0 && index < col+1)
                    p_output[i/vec_width][index-1] = p_input[j][i+t];
            }
        }
    }
}

//map from original relative position to the transformed position
//i: 0 current row, -1 last row, +1 next row
//j: 0 current col, -1 left col, +1 right col
template<int vec_width>
inline int row(int row_index, int i, int j)
{
    return row_index+j;
}

template<int vec_width>
inline int col(int col_index, int i, int j)
{
    return col_index+i*vec_width;
}


/***********************************************************************************/

/*************************Transform for stencil 3D transform********************/
//3D Transform
template<typename T, int vec_width>
void transformTo(T *input, T *output, int x, int y, int z)
{
    const int T_y = y/vec_width+2;
    const int T_z = y*vec_width;
    //output = (T*)_mm_malloc(T_row*T_col*sizeof(T), 64);
    
    T (*p_input)[y][z] = (T (*)[y][z])input;
    T (*p_output)[T_y][T_z] = (T (*)[T_y][T_z])output;
    
    //omp_set_num_threads(61);
    //#pragma omp parallel for schedule(static)
    for(int k = 0; k < x; ++k){
        for(int i = 0; i < T_z; i += vec_width){
            for(int j = 0; j < T_y; ++j){
                for(int t = 0; t < vec_width; ++t){
                    int index = j+(z/vec_width)*t;
                    if(index == 0 || index == z+1){
                        p_output[k][j][i+t] = 0;
                    }
                    else{
                        p_output[k][j][i+t] = p_input[k][i/vec_width][index-1];
                    }
                }
            }
        }
    }
}

template<typename T, int vec_width>
void transformBack(T *input, T *output, int x, int y, int z)
{
    const int T_y = y/vec_width+2;
    const int T_z = y*vec_width;
    T (*p_input)[T_y][T_z] = (T (*)[T_y][T_z])input;
    T (*p_output)[y][z] = (T (*)[y][z])output;
    
    //omp_set_num_threads(61);
    //#pragma omp parallel for schedule(static)
    for(int k = 0; k < x; ++k){
        for(int i = 0; i < T_z; i += vec_width){
            for(int j = 1; j < T_y-1; ++j){
                for(int t = 0; t < vec_width; ++t){
                    int index = j+(z/vec_width)*t;
                    if(index > 0 && index < y+1)
                        p_output[k][i/vec_width][index-1] = p_input[k][j][i+t];
                }
            }
        }
    }
}

//map from original relative position to the transformed position
//i: 0 current row, -1 last row, +1 next row
//j: 0 current col, -1 left col, +1 right col
template<int vec_width>
inline int Y(int y, int i, int j, int k)
{
    return y+k;
}

template<int vec_width>
inline int Z(int z, int i, int j, int k)
{
    return z+j*vec_width;
}

template<int vec_width>
inline int X(int x, int i, int j, int k)
{
    return x+i;
}


/***********************************************************************************/
#endif
