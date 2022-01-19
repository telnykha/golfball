#makefile for awpipl2 software 

# The name of your C compiler:
CC= gcc

# You may need to adjust these cc options:
CFLAGS= -O

# Link-time cc options:
LDFLAGS= 

#output library path 
LIB= lib/

#library source path 
SRCPATH= src/ 

INC= -Iinclude/_include/ -Iinclude

# source files: awpipl2 library proper
LIBSOURCES= awpcommon.c awpBaseAnalysis.c awpBlur.c awpCamera.c\
	awpchannels.c awpColor.c _awpColor.c awpcontour.c awpcontrast.c\
        awpconvert.c awpcopypaste.c awpdetector.c awpDistance.c awpdraw.c\
        awpEdge.c awpfft.c awpframeprocess.c awpgeometry.c\
	awpintegralfeatures.c awpInterpolation.c awpio.c _awpio.c\
	awplbp.c awpNorm.c awpopencv.c awppoints.c awpscanners.c\
	awpstroke.c awpthreshold.c awpmaskconvolution.c

# source files: awptest application 
APPSOURCE = awpmain.c 

# awpipl2 object files	
LIBOBJECTS=    awpcommon.o awpEdge.o awpBaseAnalysis.o awpBlur.o awpCamera.o\
	awpchannels.o awpColor.o _awpColor.o awpcontour.o awpcontrast.o\
	awpconvert.o awpcopypaste.o awpdetector.o awpDistance.o\
	awpdraw.o  awpfft.o awpframeprocess.o awpgeometry.o\
	awpintegralfeatures.o awpInterpolation.o\
	awpio.o _awpio.o awplbp.o awpNorm.o awpopencv.o awppoints.o\
	awpstroke.o awpthreshold.o awpmaskconvolution.o awpscanners.o

all: awpipl2.a awpmain awptest clean

awpipl:   
	$(CC) -O3 -fPIC -c $(INC) $(addprefix src/, $(LIBSOURCES)) 
awpipl2.a: awpipl
	ar cr $(LIB)awpipl2.a $(LIBOBJECTS)
	rm -f *.o *.awp 
awpmain:
	$(CC) -c $(INC) src/awpmain.c  
awptest:
	$(CC) $(INC) awpmain.o  $(LIB)awpipl2.a -ljpeg -lm -o awptest

clean:
	rm -f *.o *.awp
	
