#include<stdio.h>      /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h> 
#include<sys/stat.h>   
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>     /*错误号定义*/
#include<string.h>
#include<linux/types.h>
#include "bsp.h"
int UART0_Open(char* port)
{  
	int fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
	if (FALSE == fd)
	{
	    perror("Can't Open Serial Port");
	    return(FALSE);
	}
    //恢复串口为阻塞状态                               
    if(fcntl(fd, F_SETFL, 0) < 0)//阻塞，即使前面在open串口设备时设置的是非阻塞的，这里设为阻塞后，以此为准  
	{
		myprintf("fcntl failed!\n");
		return(FALSE);
	}     
	myprintf("串口初始化成功...\n");
	return fd;
}
void UART0_Close(int fd)
{
    close(fd);
}
int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{   
    unsigned int   i;
    int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};       
    struct termios options;  
    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
	该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    */
    if( tcgetattr(fd,&options)  !=  0)
	{
		perror("SetupSerial 0");    
		return(FALSE); 
	}
  
    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{             
			cfsetispeed(&options, speed_arr[i]); 
			cfsetospeed(&options, speed_arr[i]);  
		}
	}     
   
    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;
    //设置数据流控制
    switch(flow_ctrl)
    {
      
       case 0 ://不使用流控制
              options.c_cflag &= ~CRTSCTS;
			//  options.c_iflag |= IGNPAR;
			 // options.c_iflag &= ~(ICRNL | IXON);//避免传送二进制数据时遇到0x0d,0x11和0x13却会被丢掉
			 break;   
      
       case 1 ://使用硬件流控制
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
 	// options.c_cflag &= ~(CSIZE | PARENB);
    switch (databits)
    {  
       case 5 :
				 options.c_cflag |= CS5;
				 break;
       case 6 :
                 options.c_cflag |= CS6;
                 break;
       case 7 :    
                 options.c_cflag |= CS7;
                 break;
       case 8:    
                 options.c_cflag |= CS8;
                 break;  
       default:   
                 fprintf(stderr,"Unsupported data size\n");
                 return (FALSE); 
    }
    //设置校验位
    switch (parity)
    {  
       case 'n':
       case 'N': //无奇偶校验位。
                 options.c_cflag &= ~PARENB; 
              //   options.c_iflag &= ~INPCK;    
                 break; 
       case 'o':  
       case 'O'://设置为奇校验    
                 options.c_cflag |= PARODD;
				 options.c_cflag |= PARENB; 
                 options.c_iflag |= INPCK;             
                 break; 
       case 'e': 
       case 'E'://设置为偶校验  
                 options.c_cflag |= PARENB;       
                 options.c_cflag &= ~PARODD;       
                 options.c_iflag |= INPCK ;      
                 break;
       case 's':
       case 'S': //设置为空格 
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break; 
        default:  
                 fprintf(stderr,"Unsupported parity\n");    
                 return (FALSE); 
    } 
    // 设置停止位 
    switch (stopbits)
    {  
       case 1:   
				options.c_cflag &= ~CSTOPB; break; 
       case 2:   
				options.c_cflag |= CSTOPB; break;
       default:   
				fprintf(stderr,"Unsupported stop bits\n"); 
				return (FALSE);
    }
   
	//修改输出模式，原始数据输出
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(BRKINT | ICRNL | IXON |ISTRIP| INPCK);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);
   
    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 0; /* 读取一个字符等待1*(1/10)s */  
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */
   
    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);
   
    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
	{
		perror("com set error!\n");  
		return (FALSE); 
	}
    return (TRUE); 
}
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    //设置串口数据帧格式
    if (UART0_Set(fd,speed,flow_ctrl,databits,stopbits,parity) == FALSE)
    {                                                         
        return FALSE;
    }
    else
	{
		return  TRUE;
	}
}
int UART0_Send(int fd, void *send_buf,int data_len)
{
    int bytes_left;
	int written_bytes;
	char *ptr;
	ptr = send_buf;
	bytes_left = data_len;
	while(bytes_left > 0)
	{
	/* 开始写*/
		written_bytes = write(fd,ptr,bytes_left);
		if(written_bytes <= 0) /* 出错了*/
		{
			if(errno == EINTR) /* 中断错误 我们继续写*/
				written_bytes = 0;
			else /* 其他错误 没有办法,只好撤退了*/
				return(-1);
		}
		bytes_left -= written_bytes;
		ptr += written_bytes; /* 从剩下的地方继续写 */
	}
	return(0);
}

int UART0_Recv(int fd, void *recv_buf, int data_len)
{
	int bytes_left;
	int bytes_read;
	int err;
	char *ptr = recv_buf;
	bytes_left = data_len;
	err=0;
	while(bytes_left > 0)
	{	
		bytes_read = read(fd, ptr, bytes_left);
		if(bytes_read < 0)
		{
			if(errno == EINTR)
				bytes_read = 0;
			else
				return(-1);
		}
		else if(bytes_read == 0){
			err++;
			if(err<5)
				err++;
			else
				break;
		}
		bytes_left -= bytes_read;
		ptr += bytes_read; 
	}
	return(data_len - bytes_left);
}