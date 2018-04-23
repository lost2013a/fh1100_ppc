#include "stdio.h"
#include "CFHProSNTPWrapper.h"
#include "CUdpReceiver.h"
#include "time.h"
#include "ModelAttach.h"

/**
���߳�:sntp_service();
�����߳�: sntp����̬��

*/

MODEL_STNPD g_model_info;	//������һ��ȫ�ֵ�SNTPģ��
static void Print(MODEL_STNPD* pModel);
static int liHandler( int newLi, void* pContext);


int main(int argc,char *argv[])
{

	char    PROGRAM_DATA[40];	//����汾����
	sprintf(PROGRAM_DATA,"SNTP����汾����:%c%c%c%c-%c%c-%c%c "__TIME__"",BUILD_Y0,BUILD_Y1,BUILD_Y2,BUILD_Y3,
			BUILD_M0,BUILD_M1,BUILD_D0,BUILD_D1);
	printf("%s \r\n",PROGRAM_DATA);
	if (argc>=2){
		int tmp = atoi(argv[1]);
		tmp =~tmp;	//����������澯
	}
	
	CModelAttach attacher;			//���ü�����
	attacher.Attach(&g_model_info);	//ָ��ָ������ģ��
	/*�Զ�����һ��ģ�壬�Ա��޸Ĳ��ԣ���ʽ����ʱע��*/
	//attacher.SaveModelToXML("sntp_conf.xml");
	if (!attacher.LoadModelFromXML("sntp_conf.xml")){
		printf("SNTP��̬�����ò�����ȡʧ��!\n");
		return -1;
	}else{
		printf("SNTP��̬�����ò�����ȡ�ɹ�!\n");
		Print(&g_model_info);
	}
	CPSNTPWrapper *timeCardPtr(NULL);	
	timeCardPtr = new CPSNTPWrapper();	//������һ��SNTP���ģ��
	if (timeCardPtr){
		if (!timeCardPtr->InitLibrary()){
			printf("SNTPģ�����ʧ��\r\n");
		}
		SNTP_CONFIG &pCardInface  = g_model_info.sntpCfg;
        
		if (pCardInface.role == SNTPROLE_GPS_DEVICE){
			timeCardPtr->LeapSec(255);//4Ϊ����״̬,���⴦��
		}
		printf("SNTP��̬��������\r\n");
		if (timeCardPtr->StartSNTP(pCardInface)){					//���ö�̬��ӿ�,(����SNTP�����߳�)
			UDPRECEIVER_CONF &pInface = g_model_info.recvierCfg;	//UDP����
			CUdpReceiver *receiver = new CUdpReceiver(&pInface);	//����һ��UDPģ��
			if ((receiver)&&(0 == receiver->Init())){				//��ʼ��
				receiver->setupLiChangeCallBack(liHandler,timeCardPtr);
				receiver->Start();
				while (1){
					//��ѭ����ת
					usleep(1000);
					receiver->RecvHandle();	//����UDP����,��ȡ���е�ʱ����Ϣ(״̬�������)
				}
				receiver->End();
				delete receiver;
			}
		}
		else{
			printf("sntpЭ������ʧ��!\n");
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
	printf( "sntpCfg.debug      : %d\n", sntpCfg.debug        );   //���Ա�־                                                 
	printf( "sntpCfg.offsetUS   : %d\n", sntpCfg.offsetUS     );   //+-����ֵ��us��                                           
	printf( "sntpCfg.sntpPort   : %d\n", sntpCfg.sntpPort     );   //SNTPЭ��˿�,һ��Ϊ123�����ⲿָ���˿ڿɴ���ָ���˿�     
	printf( "sntpCfg.role       : %d\n", sntpCfg.role         );   //Ӧ�ý�ɫ����ɫ��ͬʱ�в�ͬ�����߼�                       
	printf( "recever.netType    : %d\n", recever.netType      );   //��������������                                        
	printf( "recever.loacalIp   : %s\n", recever.loacalIp     );   //����IP                                                
	printf( "recever.localPort  : %d\n", recever.localPort    );   //���ض˿�                                              
	printf( "recever.remoteIp   : %s\n", recever.remoteIp     );   //Զ��IP                                                
	printf( "recever.remotePort : %d\n", recever.remotePort   );   //Զ�̶˿�                                              
	printf( "recever.logLevel   : %d\n", recever.logLevel     );   //��־����                                              
	printf( "recever.timeOutRecv: %dms\n", recever.timeOutRecv  );   //���ճ�ʱ                                              
	printf( "recever.timeOutSend: %dms\n", recever.timeOutSend  );   //���ͳ�ʱ                                              
}


int liHandler(int newLi, void* pContext)
{
	//printf("HookHandler :%d)\n", newLi);
	if (pContext){
		CPSNTPWrapper *timeCardPtr = (CPSNTPWrapper *)pContext;
		timeCardPtr->LeapSec(newLi);	//���ö�̬������뺯��
	}
	return 0;
	
}





	

