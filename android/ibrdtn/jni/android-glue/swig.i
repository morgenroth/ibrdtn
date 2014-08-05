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
// vector support
%include "std_vector.i"
%include "stdint.i"

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

%rename(increment) operator++;
%rename(decrement) operator--;
%rename(devide) operator/;
%rename(multiply) operator*;

%rename(assignDevide) operator/=;
%rename(assignMultiply) operator*=;
%rename(operatorOr) operator|;
%rename(assignOr) operator|=;
%rename(assignAnd) operator&=;

/* ignore & operator, java does not known anything about pointer */
%ignore operator&;

/* include ibrcommon exceptions */
%typemap(javabase) ibrcommon::Exception "java.lang.Exception";
%typemap(javacode) ibrcommon::Exception %{
  public String getMessage() {
    return what();
  }
%}

/* fix memleak in byte-array handling */
%typemap(directorargout) (char *STRING, size_t LENGTH) {
    (jenv)->GetByteArrayRegion($input, 0, $2, (jbyte *)$1);
    (jenv)->DeleteLocalRef($input);
}

%include "../ibrcommon/ibrcommon/Exceptions.h"

%{
#include "../ibrdtn/ibrdtn/data/EID.h"
#include "../ibrdtn/ibrdtn/data/BundleID.h"
#include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"

#include "../ibrdtn/ibrdtn/data/SDNV.h"
#include "../ibrdtn/ibrdtn/data/Number.h"

#include "../ibrdtn/ibrdtn/data/DTNTime.h"

#include "../ibrdtn/ibrdtn/data/Block.h"
#include "../ibrdtn/ibrdtn/data/AdministrativeBlock.h"
#include "../ibrdtn/ibrdtn/data/StatusReportBlock.h"
#include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"

#include "../dtnd/src/core/EventReceiver.h"
#include "../dtnd/src/api/NativeSession.h"
#include "../dtnd/src/api/NativeSerializerCallback.h"
#include "../dtnd/src/net/NativeP2pManager.h"
#include "../dtnd/src/NativeDaemon.h"
%}

/* turn on director wrapping Callback */
%feature("director", assumeoverride=1) NativeSessionCallback;
%feature("director", assumeoverride=1) NativeSerializerCallback;
%feature("director", assumeoverride=1) NativeDaemonCallback;
%feature("director", assumeoverride=1) NativeEventCallback;
%feature("director") NativeP2pManager;

%apply unsigned long long { uint64_t };

namespace std {
    %template(StringVec) std::vector<std::string>;
}

/* Let's just grab the original header file here */
%include "../ibrdtn/ibrdtn/data/SDNV.h"
%include "../ibrdtn/ibrdtn/data/Number.h"

%template(DtnNumber) dtn::data::SDNV<dtn::data::Size>;

%include "../ibrdtn/ibrdtn/data/EID.h"
%include "../ibrdtn/ibrdtn/data/BundleID.h"
%include "../ibrdtn/ibrdtn/data/PrimaryBlock.h"

%template(PrimaryBlockFlags) dtn::data::Bitset<dtn::data::PrimaryBlock::FLAGS>;

%include "../ibrdtn/ibrdtn/data/DTNTime.h"

%include "../ibrdtn/ibrdtn/data/Block.h"

%template(BlockFlags) dtn::data::Bitset<dtn::data::Block::ProcFlags>;

%include "../ibrdtn/ibrdtn/data/AdministrativeBlock.h"
%include "../ibrdtn/ibrdtn/data/StatusReportBlock.h"
%include "../ibrdtn/ibrdtn/data/CustodySignalBlock.h"

// apply typemap for java byte[] to write()
%apply (char *STRING, size_t LENGTH) { (const char *buf, const size_t len) }

// apply typemap for java byte[] to read()
%apply char *BYTE { char *buf }
%apply int &INOUT { size_t &len }

%typemap(throws, throws="NativeDaemonException") dtn::daemon::NativeDaemonException {
  jclass excep = jenv->FindClass("de/tubs/ibr/dtn/swig/NativeDaemonException");
  if (excep)
    jenv->ThrowNew(excep, $1.what());
  return $null;
}

%typemap(throws, throws="NativeSessionException") dtn::api::NativeSessionException {
  jclass excep = jenv->FindClass("de/tubs/ibr/dtn/swig/NativeSessionException");
  if (excep)
    jenv->ThrowNew(excep, $1.what());
  return $null;
}

%typemap(throws, throws="BundleNotFoundException") dtn::api::BundleNotFoundException {
  jclass excep = jenv->FindClass("de/tubs/ibr/dtn/swig/BundleNotFoundException");
  if (excep)
    jenv->ThrowNew(excep, $1.what());
  return $null;
}

%include "../dtnd/src/core/EventReceiver.h"
%include "../dtnd/src/api/NativeSerializerCallback.h"
%include "../dtnd/src/api/NativeSession.h"
%include "../dtnd/src/net/NativeP2pManager.h"
%include "../dtnd/src/NativeDaemon.h"

