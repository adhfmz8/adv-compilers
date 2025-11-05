# CMakeLists.txt for the SimpleLICM pass

add_llvm_library(SimpleLICM MODULE
  SimpleLICM.cpp

  PLUGIN_TOOL
  opt
  )