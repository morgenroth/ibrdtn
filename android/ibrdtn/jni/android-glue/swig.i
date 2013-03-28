/* File : swig.i */
%module(directors="1") swig

// typmap for std::string
%include "std_string.i"


%{
#include "../ibrdtn/ibrdtn/data/EID.h"
#include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"
#include "../ibrdtn/ibrdtn/data/BundleID.h"
#include "../ibrdtn/ibrdtn/data/Bundle.h"
#include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"
#include "../dtnd/src/api/NativeSession.h"
%}

/* turn on director wrapping Callback */
%feature("director") NativeSessionCallback;

/* Let's just grab the original header file here */
%include "../ibrdtn/ibrdtn/data/EID.h"
%include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"

// getTimestamp() is generated two times. Ignore one
%ignore getTimestamp;
%include "../ibrdtn/ibrdtn/data/BundleID.h"
%include "../ibrdtn/ibrdtn/data/Bundle.h"
%include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"

// apply tapmap for java byte[] to write()
%apply (char *STRING, size_t LENGTH) { (const char *buf, const size_t len) }

// apply tapmap for java byte[] to write()
%apply (char *STRING, size_t LENGTH) { (char *buf, size_t &len) }
%include "../dtnd/src/api/NativeSession.h"
