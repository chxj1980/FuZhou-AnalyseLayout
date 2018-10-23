#ifndef FeatureManage_h

#include "DataDefine.h"
#include <time.h>

class CFeatureManage
{
public:
    CFeatureManage(void);
    ~CFeatureManage(void);
public:
    int Init(char * pDeviceID, int nMaxCount, LPSINGLEFEATURENODE pSingleFeatureNode);
    bool UnInit();
    //��������ֵ, bInit: �Ƿ��ڳ�ʼ��ʱ��������ֵ����, ������Ҫ�ٷ���copy���������ֵ��pFeature
    //�ص����������ֵ
    int AddFeature(LPKEYLIBFEATUREDATA pFeatureData, bool bInit = false);
    //ɾ������ֵ
    int DelFeature(const char * pFaceUUID);
    //����ص��
    int ClearKeyLib();
    unsigned int GetTotalNum();
    
private:
    LISTKEYLIBINFO m_listKeyLibInfo;  //����ֵ��, ÿ2.5w(�������ļ�ָ��)�����ݼ��ֶδ���
    LPSINGLEFEATURENODE m_pSingleFeatureNode;
    int m_nFeaturnCount;
    char m_pLibName[MAXLEN];
    int m_nMaxCount;
};

#define FeatureManage_h
#endif