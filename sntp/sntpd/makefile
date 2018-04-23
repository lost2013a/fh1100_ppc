# 府河时间测试模块
#
VPATH = -I ./ \


          
Objects =  main.o CFHProSNTPWrapper.o CUdpReceiver.o ModelAttach.o tinyxml2.o

LIBS = 

LINK =  -fPIC -lpthread -lz -ldl -lm -lrt -lnsl
																															
CFLAGS = -O2 -Wall -DOS_LINUX -fvisibility=hidden

LFLAGS  = 

CC =  $(CXX)

VERSION=1.1.3

TARGET        = ../bin/sntpd-${VERSION}
TARGET0       = sntpd

$(TARGET) : $(Objects) mklibdir
	$(CC) $(LFLAGS) $(LINK) -o $(TARGET) $(Objects)  $(LIBS) $(CFLAGS) $(VPATH) 
#	-ln -s $(TARGET) $(TARGET0)
#	mv -f $(TARGET) $(TARGET0) ../bin/
	

main.o : main.cpp
	 $(CC) -o main.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c main.cpp	   
	     
CFHProSNTPWrapper.o : CFHProSNTPWrapper.cpp
	 $(CC) -o CFHProSNTPWrapper.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c CFHProSNTPWrapper.cpp	   
	     
CUdpReceiver.o : CUdpReceiver.cpp
	 $(CC) -o CUdpReceiver.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c CUdpReceiver.cpp    
	     	  
ModelAttach.o : ModelAttach.cpp
	 $(CC) -o ModelAttach.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c ModelAttach.cpp    
	     	  
tinyxml2.o : tinyxml2.cpp
	 $(CC) -o tinyxml2.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c tinyxml2.cpp    
	     	     	     	     	      	     	     	          	     	     	     	     	     	     	     	     	     	     
mklibdir:
	if [ -d ../bin/ ]; then echo "../bin/ exists";   else mkdir -p ../bin/; fi

.PHONY : install
install:
	@echo nothing done

.PHONY : print
print:
	@echo nothing done

.PHONY : tar
tar:
	@echo nothing done

.PHONY : clean

clean :
	-rm *.o $(TARGET) $(TARGET0) $(Objects)
