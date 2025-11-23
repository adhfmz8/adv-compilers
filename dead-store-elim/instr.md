# Instructions

1. Read and understand the MemorySSADemo.cpp code and try it on some examples.

2. Compile the code -- e.g., on MacOS
`clang++ -std=c++17 -fPIC -shared MemorySSADemo.cpp -o libMemorySSADemo.dylib \
  $(llvm-config --cxxflags --ldflags | tr '\n' ' ') -lLLVM`

Compile the test examples to LLVM IR -- for example:

`clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm test/demo2.c -o test/demo2.ll

opt -passes=mem2reg test/demo2.ll -S -o test/demo2_simplified.ll`

3. Extend the MemorySSADemo.cpp code to produce a graphical view of the MemorySSA graph.
4. Implement a dead store elimination (DSE) pass following the logic of the pseudocode given in class.
5. Make up a good test set for DSE.

---

## For test cases

* pointers that must or may alias

clang++ -std=c++17 -fPIC -shared DeadStore.cpp -o libDeadStore.dylib \
  $(llvm-config --cxxflags --ldflags | tr '\n' ' ') -lLLVM

opt -load-pass-plugin ./libDeadStore.dylib \
    -passes='mem2reg,dead-store-demo' \
    -disable-output ./test/demo2.ll
