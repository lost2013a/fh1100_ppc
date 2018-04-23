#include "ModelAttach.h"

CModelAttach::CModelAttach(void)
	:m_pModel(NULL)
{

}

CModelAttach::~CModelAttach(void)
{

}


void CModelAttach::Attach( MODEL_STNPD* pModel )
{
	m_pModel = pModel;
}

tinyxml2::XMLElement* CModelAttach::BuildSNTPCfgNode(SNTP_CONFIG *senObj,tinyxml2::XMLDocument* pDoc)
{
	if (!senObj)
		return NULL;

	tinyxml2::XMLElement* element = pDoc->NewElement("SNTPCfg");
	element->SetAttribute( "debug"    , senObj->debug   );	  //���Ա�־                                               
	element->SetAttribute( "offsetUS" , senObj->offsetUS);    //+-����ֵ��us��                                        
	element->SetAttribute( "sntpPort" , senObj->sntpPort);    //SNTPЭ��˿�,һ��Ϊ123�����ⲿָ���˿ڿɴ���ָ���˿�  
	element->SetAttribute( "role"     , senObj->role    );    //Ӧ�ý�ɫ����ɫ��ͬʱ�в�ͬ�����߼�                    
	return element;     
}


tinyxml2::XMLElement* CModelAttach::BuildRecverCfgNode(UDPRECEIVER_CONF *senObj,tinyxml2::XMLDocument* pDoc)
{
	if (!senObj)
		return NULL;

	tinyxml2::XMLElement* element = pDoc->NewElement("RECEIVER_CONF");
	element->SetAttribute( "netType" ,     senObj->netType     );	//��������������  
	element->SetAttribute( "loacalIp" ,    senObj->loacalIp    );   //����IP          
	element->SetAttribute( "localPort" ,   senObj->localPort   );   //���ض˿�        
	element->SetAttribute( "remoteIp" ,    senObj->remoteIp    );   //Զ��IP          
	element->SetAttribute( "remotePort" ,  senObj->remotePort  );	//Զ�̶˿�        
	element->SetAttribute( "logLevel" ,    senObj->logLevel    );   //��־����        
	element->SetAttribute( "timeOutRecv" , senObj->timeOutRecv );   //���ճ�ʱ        
	element->SetAttribute( "timeOutSend" , senObj->timeOutSend );   //���ͳ�ʱ        

	return element;     
}

tinyxml2::XMLElement* CModelAttach::BuildsntpInfoModle( tinyxml2::XMLDocument* pDoc )
{
	tinyxml2::XMLElement* element = pDoc->NewElement("sntpInfo");
	tinyxml2::XMLElement* sntpCfgNode = BuildSNTPCfgNode(&m_pModel->sntpCfg,pDoc);
	element->LinkEndChild(sntpCfgNode);
	tinyxml2::XMLElement* RecverCfgNode = BuildRecverCfgNode(&m_pModel->recvierCfg,pDoc);
	element->LinkEndChild(RecverCfgNode);
	return element;
}

tinyxml2::XMLDeclaration* CModelAttach::BuildInfoHeader( tinyxml2::XMLDocument* pDoc )
{
	tinyxml2::XMLDeclaration* element = pDoc->NewDeclaration("version=\"1.0\" encoding=\"GBK\"");
	return element;
}

int CModelAttach::SaveModelToXML( const char* xmlFileName )
{
	if (m_pModel == NULL)	{
		return -1;
	}

	//������һ��tinyxml2::XML�ļ��������ĵ�Ҫ����д����
	tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument;
	tinyxml2::XMLDeclaration* pDelare =NULL;
	pDelare = pDoc->NewDeclaration("xml version=\"1.0\" encoding=\"GBK\" ");
	if (pDelare==NULL)		{  
		return -1;
	}
	pDoc->LinkEndChild(pDelare);
	tinyxml2::XMLElement* root = BuildsntpInfoModle(pDoc);
    pDoc->LinkEndChild(root);

	return (tinyxml2::XML_SUCCESS == pDoc->SaveFile(xmlFileName))?0:-1;
}

bool CModelAttach::LoadModelFromXML(const char* xmlFileName )
{
	if (m_pModel == NULL)	{
		return false;
	}

	tinyxml2::XMLDocument* pDoc = new tinyxml2::XMLDocument;
	tinyxml2::XMLError errNo = pDoc->LoadFile(xmlFileName);
	if (errNo != tinyxml2::XML_SUCCESS)
	{
		pDoc->PrintError();
		return false;
	}

	tinyxml2::XMLElement* sntpInfoNode = pDoc->FirstChildElement("sntpInfo");
	if (sntpInfoNode == NULL)	{
		printf("����SNTP�����ļ�����:sntpInfo �ڵ�δ�ҵ�\n");
		return false;
	}

	tinyxml2::XMLElement* sntpCfgNode = sntpInfoNode->FirstChildElement("SNTPCfg");
	if (sntpCfgNode == NULL)	{
		printf("����SNTP�����ļ�����:SNTPCfg �ڵ�δ�ҵ�\n");
		return false;
	}
	
	LoadSNTPCfgNode(&m_pModel->sntpCfg,sntpCfgNode);//����SNTPCfg �ڵ�

	tinyxml2::XMLElement* RecverCfgNode = sntpInfoNode->FirstChildElement("RECEIVER_CONF");
	if (RecverCfgNode == NULL)	{
		printf("����SNTP�����ļ�����:RECEIVER_CONF �ڵ�δ�ҵ�\n");
		return false;
	}
	return LoadRecverCfgNode(&m_pModel->recvierCfg,RecverCfgNode);
}


bool CModelAttach::LoadSNTPCfgNode(SNTP_CONFIG *senObj,tinyxml2::XMLElement* element)
{
	if (!senObj)
		return false;

	const char* elemAttr=NULL;
	elemAttr = element->Attribute( "debug"   ); if (elemAttr) { senObj->debug    = atoi( elemAttr ); }
	elemAttr = element->Attribute( "offsetUS"); if (elemAttr) { senObj->offsetUS = atoi( elemAttr ); }
	elemAttr = element->Attribute( "sntpPort"); if (elemAttr) { senObj->sntpPort = atoi( elemAttr ); }
	elemAttr = element->Attribute( "role"    ); if (elemAttr) { senObj->role     = atoi( elemAttr ); }
	return true;
}

bool CModelAttach::LoadRecverCfgNode( UDPRECEIVER_CONF *senObj,tinyxml2::XMLElement* element )
{
	if (!senObj)
		return false;

	const char* elemAttr=NULL;
	elemAttr = element->Attribute( "netType"     ); if (elemAttr) { senObj->netType      = atoi( elemAttr ); }
	elemAttr = element->Attribute( "loacalIp"    ); if (elemAttr) { strncpy(senObj->loacalIp, elemAttr, 40); }
	elemAttr = element->Attribute( "localPort"   ); if (elemAttr) { senObj->localPort    = atoi( elemAttr ); }
	elemAttr = element->Attribute( "remoteIp"    ); if (elemAttr) { strncpy(senObj->remoteIp, elemAttr, 40); }
	elemAttr = element->Attribute( "remotePort"  ); if (elemAttr) { senObj->remotePort   = atoi( elemAttr ); }
	elemAttr = element->Attribute( "logLevel"    ); if (elemAttr) { senObj->logLevel     = atoi( elemAttr ); }
	elemAttr = element->Attribute( "timeOutRecv" ); if (elemAttr) { senObj->timeOutRecv  = atoi( elemAttr ); }
	elemAttr = element->Attribute( "timeOutSend" ); if (elemAttr) { senObj->timeOutSend  = atoi( elemAttr ); }
	return true;
}
