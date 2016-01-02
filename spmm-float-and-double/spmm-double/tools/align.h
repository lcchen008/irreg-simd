#ifndef ALIGN_H
#define ALIGN_H 

// Calculate the size in aligned fashion.  
template<class ElementType>
// vlen: block size to be aligned with.
// n: total number of elements to be allocated.
int align(int vlen, int n) {
  int align_size = vlen/sizeof(ElementType);
  return (n + align_size - 1)&(~(align_size - 1));
}

#endif /* ALIGN_H */
