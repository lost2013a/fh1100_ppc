#include<stdio.h>      /*��׼�����������*/
#include<stdlib.h>     /*��׼�����ⶨ��*/
#include<unistd.h>     /*Unix ��׼��������*/
#include<sys/types.h> 
#include<sys/stat.h>   
#include<fcntl.h>      /*�ļ����ƶ���*/
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/
#include<errno.h>     /*����Ŷ���*/
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
    //�ָ�����Ϊ����״̬                               
    if(fcntl(fd, F_SETFL, 0) < 0)//��������ʹǰ����open�����豸ʱ���õ��Ƿ������ģ�������Ϊ�������Դ�Ϊ׼  
	{
		myprintf("fcntl failed!\n");
		return(FALSE);
	}     
	myprintf("���ڳ�ʼ���ɹ�...\n");
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
    /*tcgetattr(fd,&options)�õ���fdָ��������ز������������Ǳ�����options,
	�ú��������Բ��������Ƿ���ȷ���ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1.
    */
    if( tcgetattr(fd,&options)  !=  0)
	{
		perror("SetupSerial 0");    
		return(FALSE); 
	}
  
    //���ô������벨���ʺ����������
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
	{
		if  (speed == name_arr[i])
		{             
			cfsetispeed(&options, speed_arr[i]); 
			cfsetospeed(&options, speed_arr[i]);  
		}
	}     
   
    //�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���
    options.c_cflag |= CLOCAL;
    //�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������
    options.c_cflag |= CREAD;
    //��������������
    switch(flow_ctrl)
    {
      
       case 0 ://��ʹ��������
              options.c_cflag &= ~CRTSCTS;
			//  options.c_iflag |= IGNPAR;
			 // options.c_iflag &= ~(ICRNL | IXON);//���⴫�Ͷ���������ʱ����0x0d,0x11��0x13ȴ�ᱻ����
			 break;   
      
       case 1 ://ʹ��Ӳ��������
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://ʹ�����������
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //��������λ
    //����������־λ
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
    //����У��λ
    switch (parity)
    {  
       case 'n':
       case 'N': //����żУ��λ��
                 options.c_cflag &= ~PARENB; 
              //   options.c_iflag &= ~INPCK;    
                 break; 
       case 'o':  
       case 'O'://����Ϊ��У��    
                 options.c_cflag |= PARODD;
				 options.c_cflag |= PARENB; 
                 options.c_iflag |= INPCK;             
                 break; 
       case 'e': 
       case 'E'://����ΪżУ��  
                 options.c_cflag |= PARENB;       
                 options.c_cflag &= ~PARODD;       
                 options.c_iflag |= INPCK ;      
                 break;
       case 's':
       case 'S': //����Ϊ�ո� 
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break; 
        default:  
                 fprintf(stderr,"Unsupported parity\n");    
                 return (FALSE); 
    } 
    // ����ֹͣλ 
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
   
	//�޸����ģʽ��ԭʼ�������
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(BRKINT | ICRNL | IXON |ISTRIP| INPCK);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);
   
    //���õȴ�ʱ�����С�����ַ�
    options.c_cc[VTIME] = 0; /* ��ȡһ���ַ��ȴ�1*(1/10)s */  
    options.c_cc[VMIN] = 1; /* ��ȡ�ַ������ٸ���Ϊ1 */
   
    //�����������������������ݣ����ǲ��ٶ�ȡ ˢ���յ������ݵ��ǲ���
    tcflush(fd,TCIFLUSH);
   
    //�������� (���޸ĺ��termios�������õ������У�
    if (tcsetattr(fd,TCSANOW,&options) != 0)  
	{
		perror("com set error!\n");  
		return (FALSE); 
	}
    return (TRUE); 
}
int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    //���ô�������֡��ʽ
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
	/* ��ʼд*/
		written_bytes = write(fd,ptr,bytes_left);
		if(written_bytes <= 0) /* ������*/
		{
			if(errno == EINTR) /* �жϴ��� ���Ǽ���д*/
				written_bytes = 0;
			else /* �������� û�а취,ֻ�ó�����*/
				return(-1);
		}
		bytes_left -= written_bytes;
		ptr += written_bytes; /* ��ʣ�µĵط�����д */
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