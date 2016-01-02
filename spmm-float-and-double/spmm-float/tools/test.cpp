#include <iostream>
using namespace std;

#include "csr.h"
#include "csr_loader.h"

int main() {
  // Test alignment.
  int i = 500;
  int aligned = align<int>(64, i);
  cout << "Aligned: " << aligned << endl; 
  Csr<float>* csr = CsrLoader<float>::AlignedLoad("../datasets/test"); 
  cout << "Load done." << endl;
  csr->print();
  delete csr;
  float a[16];
  cout << &a << " " << &a[0] << endl;
  return 0;
}
