#release
gcc -O3 -DNDEBUG -DITHARE_OBF_SEED=1234567 -DITHARE_OBF_DBG_RUNTIME_CHECKS -DITHARE_OBF_ENABLE_AUTO_DBGPRINT -o obfofficial -std=c++1z -lstdc++ ../official.cpp 

