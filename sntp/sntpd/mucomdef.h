#ifndef MUCOMDEF_H_
#define MUCOMDEF_H_

#ifdef __cplusplus
#define BEGIN_C_DECLS  extern "C" {
#define END_C_DECLS      } 
#else
#define BEGIN_C_DECLS
#define END_C_DECLS 
#endif


#if defined(WIN32) 
#define COM_DATA_EXPORT __declspec(dllexport)
#else
#define COM_DATA_EXPORT
#endif

#define  COMDEF_INTER_UDP_COM_RECEIVE_TIMEOUT  5000      //�ڲ�UDP���ճ�ʱʱ��
#define  COMDEF_INTER_MAX_BUFFER_LENGTH        5000
#define  COMDEF_CAST_INFO     0xFFFF

//������ֵ
#define  COMDEF_CMD_RESULT_FALSE          0
#define  COMDEF_CMD_RESULT_SUCCESS        1 

//��������
#define  COMDEF_CMD_RESULT                1  //��������Ӧ
#define  COMDEF_CMD_VAL                   2  //����ֵ 
#define  COMDEF_CMD_EVENT                 3  //�¼���Ϣ  

//�������ݽṹ

#pragma pack (push, 1)
//����ͷ�ļ�
typedef struct tagcomdef_head
{
	unsigned short  from;       //���������Ķ�
    unsigned short  to;         //���ķ����Ķ�
	unsigned int    equipid;    //װ������
	unsigned short  comtype;    //���������

}COMDEF_HEAD;

typedef struct tagcomdef_val
{
   unsigned int  infoindex;
   unsigned char datatype;
   unsigned int  value; 

} COMDEF_VAL;

typedef struct tagcomdef_event 
{
 
   unsigned int   infoindex;                                //��Ϣ����
   char           time[32];
   unsigned char  datatype;                                 //ֵ��������
   int            value;   //ֵ
}COMDEF_EVENT;








#pragma pack(pop)



#endif