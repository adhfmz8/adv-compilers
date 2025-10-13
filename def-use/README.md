# How I ran this

`clang++ -fPIC -shared DefUseChains.cpp -o DefUseChains.dylib \
-I$(brew --prefix llvm)/include \
-L$(brew --prefix llvm)/lib \
-Wl,-rpath,$(brew --prefix llvm)/lib \ `

I put gcd_canonical.ll to test

`opt -load-pass-plugin=./DefUseChains.dylib -passes="def-use-chains" -disable-output gcd_canonical.ll`

clang++ -fPIC -shared LoopInfoExample.cpp -o LoopInfoExample.dylib \
-I$(brew --prefix llvm)/include \
-L$(brew --prefix llvm)/lib \
-Wl,-rpath,$(brew --prefix llvm)/lib \
-lLLVM

`opt -load-pass-plugin=./LoopInfoExample.dylib -passes="loop-info-example" -disable-output gcd_canonical.ll`

## Example output:

Analyzing function: gcd
Loop at depth1
with preheader: %.lr.ph
with latch: %4
with header: %4