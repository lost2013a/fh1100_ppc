.PHONY:clean
#CC = gcc	//编译环境中已定义
#CFLAGS=-Wall  -O2
OBJ = rmcrc_ok
#OBJ = test
CFLAGS=-Wall -O2
LIBS=-lpthread
OBJS = bsp.c serial.c serial_init.c config.c log.c udp.c package_log_config.c mycan.c can_init.c package_dyn_can.c			

$(OBJ):$(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LIBS)

clean:
	rm  $(OBJ) 
