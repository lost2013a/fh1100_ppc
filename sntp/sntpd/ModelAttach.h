#ifndef _ModelAttach_H
#define _ModelAttach_H

#include "model.h"
#include "tinyxml2.h"

#define WIN32_LEAN_AND_MEAN
using namespace tinyxml2;

class CModelAttach
{
public:
	CModelAttach(void);
	virtual ~CModelAttach(void);
	void Attach(MODEL_STNPD* pModel);
	int  SaveModelToXML(const char* FileName );
	tinyxml2::XMLElement* BuildsntpInfoModle(tinyxml2::XMLDocument* pDoc);
	tinyxml2::XMLDeclaration* BuildInfoHeader(tinyxml2::XMLDocument* pDoc);
	tinyxml2::XMLElement* BuildSNTPCfgNode(SNTP_CONFIG *pNode,tinyxml2::XMLDocument* pDoc);
	tinyxml2::XMLElement* BuildRecverCfgNode(UDPRECEIVER_CONF *senObj,tinyxml2::XMLDocument* pDoc);
	bool LoadModelFromXML(const char* FileName );
	bool LoadSNTPCfgNode(SNTP_CONFIG *senObj,tinyxml2::XMLElement* element);
	bool LoadRecverCfgNode(UDPRECEIVER_CONF *senObj,tinyxml2::XMLElement* element);
private:
	MODEL_STNPD* m_pModel;	
};
#endif // _ModelAttach_H