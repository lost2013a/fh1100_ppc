# 府河SNTP服务端规约模块
#
VPATH = -I../ \
        -I../common \
        
Objects = FHSNTPInface.o SNTPMainFlow.o sntpMsgAttach.o LogFile.o platform.o

LIBS =  
LINK = -fPIC -lpthread -lz -ldl -lm -lrt -lnsl   																															
CFLAGS = -O2 -w -DOS_LINUX -fvisibility=hidden -mlongcall
#-mlongcall  ->PowerPC平台下使用长跳转

VERSION=1.1.3

CC =  $(CXX)

TARGET        = libFHTIMESNTP.so-${VERSION}
TARGET0       = libFHTIMESNTP.so

libFHTIMESNTP.so-${VERSION} : $(Objects) mklibdir
	 $(CC) $(LFLAGS) -shared $(LINK) -o $(TARGET) $(Objects)  $(LIBS) $(CFLAGS) $(VPATH)
	-ln -s $(TARGET) $(TARGET0)
	mv -f $(TARGET) $(TARGET0) ../bin/

FHSNTPInface.o : FHSNTPInface.cpp
	 $(CC) -o FHSNTPInface.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c FHSNTPInface.cpp	     
	     
SNTPMainFlow.o : SNTPMainFlow.cpp
	 $(CC) -o SNTPMainFlow.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c SNTPMainFlow.cpp	   
	     	     
sntpMsgAttach.o : sntpMsgAttach.cpp
	 $(CC) -o sntpMsgAttach.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c sntpMsgAttach.cpp	   

LogFile.o : ../common/LogFile.cpp
	 $(CC) -o LogFile.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c ../common/LogFile.cpp	

platform.o : ../common/platform.cpp
	 $(CC) -o platform.o $(LIBS) $(CFLAGS) $(VPATH) \
	     -c ../common/platform.cpp	

	     	     	     	     	     	     	     	     	     	     	     	     	     	     	     	     	     
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
