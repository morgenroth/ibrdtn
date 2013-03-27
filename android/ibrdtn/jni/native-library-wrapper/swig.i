/* File : swig.i */
%module swig

%{
#include "../dtnd/src/api/NativeSession.h"
%}

/* Let's just grab the original header file here */
%include "../dtnd/src/api/NativeSession.h"
 
