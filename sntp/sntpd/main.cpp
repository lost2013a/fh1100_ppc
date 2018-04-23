#include "stdio.h"
#include "CFHProSNTPWrapper.h"
#include "CUdpReceiver.h"
#include "time.h"
#include "ModelAttach.h"

/**
主线程:sntp_service();
调用线程: sntp服务动态库

*/

MODEL_STNPD g_model_info;	//定义了一个全局的SNTP模型
static void Print(MODEL_STNPD* pModel);
static int liHandler( int newLi, void* pContext);


int main(int argc,char *argv[])
{

	char    PROGRAM_DATA[40];	//程序版本日期
	sprintf(PROGRAM_DATA,"SNTP程序版本日期:%c%c%c%c-%c%c-%c%c "__TIME__"",BUILD_Y0,BUILD_Y1,BUILD_Y2,BUILD_Y3,
			BUILD_M0,BUILD_M1,BUILD_D0,BUILD_D1);
	printf("%s \r\n",PROGRAM_DATA);
	if (argc>=2){
		int tmp = atoi(argv[1]);
		tmp =~tmp;	//避免编译器告警
	}
	
	CModelAttach attacher;			//配置加载器
	attacher.Attach(&g_model_info);	//指针指向配置模型
	/*自动生成一个模板，以便修改测试，正式运行时注释*/
	//attacher.SaveModelToXML("sntp_conf.xml");
	if (!attacher.LoadModelFromXML("sntp_conf.xml")){
		printf("SNTP动态库配置参数获取失败!\n");
		return -1;
	}else{
		printf("SNTP动态库配置参数获取成功!\n");
		Print(&g_model_info);
	}
	CPSNTPWrapper *timeCardPtr(NULL);	
	timeCardPtr = new CPSNTPWrapper();	//申请了一个SNTP外壳模型
	if (timeCardPtr){
		if (!timeCardPtr->InitLibrary()){
			printf("SNTP模块加载失败\r\n");
		}
		SNTP_CONFIG &pCardInface  = g_model_info.sntpCfg;
        
		if (pCardInface.role == SNTPROLE_GPS_DEVICE){
			timeCardPtr->LeapSec(255);//4为特殊状态,特殊处理
		}
		printf("SNTP动态库加载完成\r\n");
		if (timeCardPtr->StartSNTP(pCardInface)){					//调用动态库接口,(建立SNTP服务线程)
			UDPRECEIVER_CONF &pInface = g_model_info.recvierCfg;	//UDP配置
			CUdpReceiver *receiver = new CUdpReceiver(&pInface);	//申请一个UDP模型
			if ((receiver)&&(0 == receiver->Init())){				//初始化
				receiver->setupLiChangeCallBack(liHandler,timeCardPtr);
				receiver->Start();
				while (1){
					//主循环空转
					usleep(1000);
					receiver->RecvHandle();	//处理UDP报文,提取其中的时间信息(状态，闰秒等)
				}
				receiver->End();
				delete receiver;
			}
		}
		else{
			printf("sntp协议启动失败!\n");
		}
		timeCardPtr->EndSNTP();
		delete timeCardPtr;
	}
	return 0;
}


void Print(MODEL_STNPD* pModel)
{
	SNTP_CONFIG &sntpCfg = pModel->sntpCfg;
	UDPRECEIVER_CONF& recever = pModel->recvierCfg;
	printf( "sntpCfg.debug      : %d\n", sntpCfg.debug        );   //调试标志                                                 
	printf( "sntpCfg.offsetUS   : %d\n", sntpCfg.offsetUS     );   //+-补偿值（us）                                           
	printf( "sntpCfg.sntpPort   : %d\n", sntpCfg.sntpPort     );   //SNTP协议端口,一般为123，若外部指定端口可传入指定端口     
	printf( "sntpCfg.role       : %d\n", sntpCfg.role         );   //应用角色，角色不同时有不同处理逻辑                       
	printf( "recever.netType    : %d\n", recever.netType      );   //接收者网络类型                                        
	printf( "recever.loacalIp   : %s\n", recever.loacalIp     );   //本地IP                                                
	printf( "recever.localPort  : %d\n", recever.localPort    );   //本地端口                                              
	printf( "recever.remoteIp   : %s\n", recever.remoteIp     );   //远程IP                                                
	printf( "recever.remotePort : %d\n", recever.remotePort   );   //远程端口                                              
	printf( "recever.logLevel   : %d\n", recever.logLevel     );   //日志级别                                              
	printf( "recever.timeOutRecv: %dms\n", recever.timeOutRecv  );   //接收超时                                              
	printf( "recever.timeOutSend: %dms\n", recever.timeOutSend  );   //发送超时                                              
}


int liHandler(int newLi, void* pContext)
{
	//printf("HookHandler :%d)\n", newLi);
	if (pContext){
		CPSNTPWrapper *timeCardPtr = (CPSNTPWrapper *)pContext;
		timeCardPtr->LeapSec(newLi);	//调用动态库的闰秒函数
	}
	return 0;
	
}





	

