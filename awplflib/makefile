#makefile for awplflib software 

# The name of your C compiler:

CC= cc

# You may need to adjust these cc options:
CFLAGS= -O

# Link-time cc options:
LDFLAGS= 

#output library path 
LIB    = lib/
#awpipl2 library path 
AWPLIB = ../awpipl2/lib/

#library source path 
SRCPATH= src/ 

#roc path 
APPROC = utils/roc/

#builder path 
APPBUILDER = utils/builder/


INC= -Iinclude -I../awpipl2/include

# source files: awplflib library proper
LIBSOURCES=  LFAttrFilter.cpp \
	 LFBuilder.cpp LFBuffer.cpp LFCluster.cpp LFColorDetector.cpp\
        LFCommon.cpp lfcsboost.cpp LFCSFeature.cpp LFCSFeature.cpp LFDetectedItem.cpp\
        LFDetector.cpp LFDistance.cpp LFEngine.cpp LFEye.cpp LFFaceDescriptor.cpp\
	LFFeatureImpl.cpp LFFeatures.cpp LFFileUtils.cpp LFGeometry.cpp LFHistogramm.cpp\
	LFImage.cpp LFImageProcessor.cpp LFLabColor.cpp LFMotion.cpp \
	LFParameter.cpp LFPredictors.cpp LFSample.cpp LFScanners.cpp LFSemantic.cpp\
	LFStrong.cpp LFStrongImpl.cpp LFThresholdProc.cpp LFUtils.cpp LFVector.cpp\
	LFWeak.cpp LFWeakImpl.cpp LFZones.cpp LFDatabase.cpp LFAvgFeature.cpp LFSabotage.cpp

LIBOBJECTS=  LFAttrFilter.o \
	 LFBuilder.o LFBuffer.o LFCluster.o LFColorDetector.o\
        LFCommon.o lfcsboost.o LFCSFeature.o LFCSFeature.o LFDetectedItem.o\
        LFDetector.o LFDistance.o LFEngine.o LFEye.o LFFaceDescriptor.o\
	LFFeatureImpl.o LFFeatures.o LFFileUtils.o LFGeometry.o LFHistogramm.o\
	LFImage.o LFImageProcessor.o LFLabColor.o LFMotion.o \
	LFParameter.o LFPredictors.o LFSample.o LFScanners.o LFSemantic.o\
	LFStrong.o LFStrongImpl.o LFThresholdProc.o LFUtils.o LFVector.o\
	LFWeak.o LFWeakImpl.o LFZones.o LFDatabase.o LFAvgFeature.o LFSabotage.o

all: awplflib.a rocmain roc engmain eng buildmain builder clean

awplflib:   
	$(CC)  -fPIC -Ofast -c $(INC) $(addprefix src/, $(LIBSOURCES)) 
awplflib.a: awplflib
	ar cr $(LIB)awplflib.a $(LIBOBJECTS)
rocmain:
	$(CC) -c $(INC) $(APPROC)roc.cpp 
engmain:
	$(CC) -c $(INC) eng.cpp 
buildmain: 
	$(CC) -c $(INC) $(APPBUILDER)main.cpp 
#$(LIB)awplflib.a
roc:
	g++ $(INC) roc.o $(LIB)awplflib.a $(AWPLIB)awpipl2.a -ljpeg -luuid -ltinyxml -o roc
eng:
	g++ $(INC) eng.o $(LIB)awplflib.a $(AWPLIB)awpipl2.a -ljpeg -luuid -ltinyxml -o eng
builder:
	g++ $(INC) main.o $(LIB)awplflib.a $(AWPLIB)awpipl2.a -ljpeg -luuid -ltinyxml -o csbuild

clean:
	rm -f *.o *.awp
