rm -Rf ../src/de/tubs/ibr/dtn/swig
mkdir -p ../src/de/tubs/ibr/dtn/swig
swig -c++ -java -package de.tubs.ibr.dtn.swig -verbose -outdir ../src/de/tubs/ibr/dtn/swig/ -o android-glue/SWIGWrapper.cpp android-glue/swig.i 
