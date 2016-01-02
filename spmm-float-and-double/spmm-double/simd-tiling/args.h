#ifndef ARGS_H
#define ARGS_H 

#include "tiled_csr.h"

#include <mutex>

template<class ValueType>
struct args {
  int* task_offset; // For task scheduling.
  int total_tasks; // Total rows.
  TiledCsr<ValueType>* m1;
  TiledCsr<ValueType>* m2;
};

#endif /* ARGS_H */
