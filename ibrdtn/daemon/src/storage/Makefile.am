## sub directory

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

storage_SOURCES =	\
				BundleStorage.cpp \
				BundleStorage.h \
				MemoryBundleStorage.h \
				MemoryBundleStorage.cpp \
				SimpleBundleStorage.cpp \
				SimpleBundleStorage.h \
				DataStorage.h \
				DataStorage.cpp
				
if SQLITE
storage_SOURCES += SQLiteConfigure.h SQLiteConfigure.cpp SQLiteBundleStorage.h SQLiteBundleStorage.cpp SQLiteDatabase.h SQLiteDatabase.cpp
AM_CPPFLAGS += @SQLITE_CFLAGS@
AM_LDFLAGS += @SQLITE_LIBS@
endif

noinst_LTLIBRARIES = libstorage.la
libstorage_la_SOURCES= $(storage_SOURCES)
