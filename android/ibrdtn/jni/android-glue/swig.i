/* File : swig.i */
%module(directors="1") swig

// typemap for std::exception
%include "std_except.i"
// typemap for std::string
%include "std_string.i"
// typemap for BYTE
%include "various.i"
// INPUT, OUTPUT, INOUT typemaps
%include "typemaps.i"

/* add renaming directive for standard operators */
%rename(assign) operator=;
%rename(equals) operator==;
%rename(notEquals) operator!=;
%rename(add) operator+;
%rename(addAssign) operator+=;
%rename(sub) operator-;
%rename(subAssign) operator-=;
%rename(lowerThan) operator<;
%rename(greaterThan) operator>;
%rename(serialize) operator<<;
%rename(deserialize) operator>>;
%rename(lowerOrEqualThan) operator<=;
%rename(greaterOrEqualThan) operator>=;

/* ignore & operator, java does not known anything about pointer */
%ignore operator&;

/* include ibrcommon exceptions */
%include "../ibrcommon/ibrcommon/Exceptions.h"

%{
#include "../ibrdtn/ibrdtn/data/EID.h"
#include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"

#include "../ibrdtn/ibrdtn/data/SDNV.h"

#include "../ibrdtn/ibrdtn/data/DTNTime.h"


#include "../ibrdtn/ibrdtn/data/BundleID.h"

#include "../ibrdtn/ibrdtn/data/AdministrativeBlock.h"
#include "../ibrdtn/ibrdtn/data/StatusReportBlock.h"
#include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"

#include "../dtnd/src/api/NativeSession.h"
%}

/* turn on director wrapping Callback */
%feature("director") NativeSessionCallback;

/* Let's just grab the original header file here */
%include "../ibrdtn/ibrdtn/data/EID.h"
%include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"

%include "../ibrdtn/ibrdtn/data/SDNV.h"

%include "../ibrdtn/ibrdtn/data/DTNTime.h"

%include "../ibrdtn/ibrdtn/data/BundleID.h"

%include "../ibrdtn/ibrdtn/data/AdministrativeBlock.h"
%include "../ibrdtn/ibrdtn/data/StatusReportBlock.h"
%include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"

// apply typemap for java byte[] to write()
%apply (char *STRING, size_t LENGTH) { (const char *buf, const size_t len) }

// apply typemap for java byte[] to read()
%apply char *BYTE { char *buf }
%apply int &INOUT { size_t &len }

%include "../dtnd/src/api/NativeSession.h"
