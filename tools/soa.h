#ifndef SOA_H
#define SOA_H 

// Struct of arrays for moldyn 3D data.
class ThreeDSoa {
 public:
  int num_nodes;
  float* x;
  float* y;
  float* z;

  ThreeDSoa (int num_nodes) {
    this->num_nodes = num_nodes;
    x = (float*)_mm_malloc(sizeof(float)*num_nodes, 64);  
    y = (float*)_mm_malloc(sizeof(float)*num_nodes, 64);  
    z = (float*)_mm_malloc(sizeof(float)*num_nodes, 64);  
  } 

  void SetXyz(const vector<tuple<float, float, float> >& xyz) {
    for (int i = 0; i < num_nodes; ++i) {
      x[i] = get<0>(xyz[i]);
      y[i] = get<1>(xyz[i]);
      z[i] = get<2>(xyz[i]);
    }
  }

  ~ThreeDSoa() {
    _mm_free(x);
    _mm_free(y);
    _mm_free(z);
  }
};

#endif /* SOA_H */
