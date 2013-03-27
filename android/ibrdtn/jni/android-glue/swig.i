/* File : swig.i */
%module(directors="1") swig

%{
#include "../ibrdtn/ibrdtn/data/EID.h"
#include "../ibrdtn/ibrdtn/data/BundleID.h"
#include "../ibrdtn/ibrdtn/data/Bundle.h"
#include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"
#include "../dtnd/src/api/NativeSession.h"
%}

%include "std_string.i"

/* turn on director wrapping Callback */
%feature("director") NativeSessionCallback;

/* Let's just grab the original header file here */
%include "../ibrdtn/ibrdtn/data/EID.h"
%include "../ibrdtn/ibrdtn/data/BundleID.h"
%include "../ibrdtn/ibrdtn/data/Bundle.h"
%include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"
%include "../dtnd/src/api/NativeSession.h"
 
