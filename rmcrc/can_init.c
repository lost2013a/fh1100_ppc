#include "can_init.h"
#include <time.h>

int  socket_fd = -1;

// -----------------------------------------------------------------------------
 void panic(const char *msg)
{
    printf("%s\n", msg);
    return ;
}

void delay_ms(const __u32 ms)
{
    struct timespec tv;
    tv.tv_sec = ms / 1000;//change to second
    tv.tv_nsec = (long)(ms % 1000) * 1000000;
    nanosleep(&tv, NULL);
}

//------------------------------------------------------------------------
//can总线操作
//------------------------------------------------------------------------
#if 0
bool find_can(const int port)
{
    char buf[128] = {0};
    sprintf(buf, "/sys/class/net/can%d", port);
    return  ((access(buf, 0) == 0));
}

/*
 *	bitrate: 250000 or 125000
 */
void set_bitrate(const int port, const int bitrate)
{
#define TX_QUEUE_LEN		4096 // 使用足够多的发送缓存

	char	l_c8Command[128] = {0};
	sprintf(l_c8Command, "echo %d > /sys/class/net/can%d/can_bittiming/bitrate", bitrate, port);
	system(l_c8Command);//执行shell命令也就是向dos发送一条指令

	// 设置tx_queue_len
	memset(l_c8Command, 0, sizeof(l_c8Command));
	sprintf(l_c8Command, "echo %d > /sys/class/net/can%d/tx_queue_len", TX_QUEUE_LEN, port);
	system(l_c8Command);

}
#endif

void open_can(const int port, const int bitrate)
{
    char	l_c8Command[64] = {0};
	sprintf(l_c8Command, "/mnt/HD/ip link set can%d up type can bitrate %d", port, bitrate);
	system(l_c8Command);

}


void close_can(const int port)
{
    char	l_c8Command[64] = {0};
    sprintf(l_c8Command, "/mnt/HD/ip link set can%d down", port);
    system(l_c8Command);
}

void close_socket(const int sockfd)
{
    if (sockfd != -1)
    {
        close(sockfd);
    }
}




// 绑定sock，然后监听端口
// 返回监听 套接字 文件描述符
int socket_listen(const int port)
{
    int sockfd = -1;
    struct sockaddr_can _addr;
    struct ifreq _ifreq;
    char buf[256];
    int ret = 0;

     /* 建立套接字，设置为原始套接字，原始CAN协议 */
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0) 
    {
        sprintf(buf, "\n\t打开 socket can%d 错误\n\n", port + 1);
        panic(buf);
        return  -1;
    }

     /* 以下是对CAN接口进行初始化，如设置CAN接口名，即当我们用ifconfig命令时显示的名字 */
    sprintf(buf, "can%d", port);
    strcpy(_ifreq.ifr_name, buf);
    ret = ioctl(sockfd, SIOCGIFINDEX, &_ifreq);
    if (ret < 0) 
    {
        sprintf(buf, "\n\t匹配 socket can%d 错误\n\n", port + 1);
        panic(buf);
        return  -1;
    }

    /* 设置CAN协议 */
    _addr.can_family = AF_CAN;
    _addr.can_ifindex = _ifreq.ifr_ifindex;


    /* disable default receive filter on this RAW socket */
    /* This is obsolete as we do not read from the socket at all, but for */
    /* this reason we can remove the receive list in the Kernel to save a */
    /* little (really a very little!) CPU usage.                          */
    //	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
	
	struct can_filter rfilter[1];
	rfilter[0].can_id = 0x03100000;		//只筛选0x0310的报文;
	rfilter[0].can_mask = 0x1FF00000;	//CAN_EFF_MASK
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter, sizeof(rfilter));//设置规则
    /* 将刚生成的套接字与CAN套接字地址进行绑定 */
    ret = bind(sockfd, (struct sockaddr *)&_addr, sizeof(_addr));
    if ( ret < 0) 
    {
        close_socket(sockfd);
        sprintf(buf, "\n\t绑定 socket can%d 错误\n\n", port + 1);
        panic(buf);
        return  -1;
    }
    return  sockfd;
}
/*
int set_can_loopback(const int sockfd, const bool lp)
{
    // 在默认情况下，本地回环功能是开启的，可以使用下面的方法关闭回环/开启功能：
    int loopback = lp;  // 0表示关闭, 1表示开启(默认) 
    (void)(sockfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
    return  0;
}
*/

int socket_connect(const int port)
{
    return  socket_listen(port);
}

void disconnect(int *sockfd)
{
    if (!sockfd || *sockfd == -1)
    {
        return ;
    }
    close_socket(*sockfd);
    *sockfd = -1;
}

/*int send_frame(const int sockfd, const byte* data, const int count)
{
    int ret = write(sockfd, (const char*)data, count);
    if (ret > 0)
    {
        tcdrain(sockfd);//不做出错处理，因为网络不支持tcdrain//Tcdrain  等待所有输出都被传输
    }
    return  ret;
}

int recv_frame(const int sockfd, S_CanFrame* buf, const int count, const int timeout_ms)
{
    struct timeval tv_timeout;
    tv_timeout.tv_sec  = timeout_ms  / 1000;
    tv_timeout.tv_usec = (timeout_ms % 1000) * 1000;
    fd_set fs_read;
    
    FD_ZERO(&fs_read);
    FD_SET(sockfd, &fs_read);	//if fd == -1, FD_SET will block here

    int ret = select((int)sockfd + 1, &fs_read, NULL, NULL, &tv_timeout);
    if (ret == 0) // recv timeout
    {
        return  0;
    }
    if (ret < 0) // select error
    {
        return  ret;
    }

    ret = read(sockfd, (char*)buf, count);

    if (ret <= 0)
    {
        return  -1;
    }
    return  ret;
}
*/
