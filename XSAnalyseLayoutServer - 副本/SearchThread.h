#ifndef searchthread_h


#include "ZeromqManage.h"
class CSearchThread
{
public:
    CSearchThread();
    ~CSearchThread();
public:
    bool Init(char * pServerIP, int nServerPushPort, int nServerPullPort, LPALLFEATURENODEINFO pAllFeatureNodeInfo);
    bool UnInit();
private:
    //zeromq������Ϣ�ص�
    static void ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
    //����Zeromq json
    bool ParseZeromqJson(LPSUBMESSAGE pSubMessage);
    void VerifyFeatureAction(LPSUBMESSAGE pSubMessage);
    int VerifyFeature(unsigned char * pf1, unsigned char * pf2, int nLen);

    //���ͻ�Ӧ��Ϣ
    void SendResponseMsg(LPSUBMESSAGE pSubMessage);
private:
    CZeromqManage * m_pZeromqManage;    //Zeromq����
    static int m_nThreadNum;
    int m_nThreadID;
    LPALLFEATURENODEINFO m_pAllFeatureNodeInfo;
};

#define searchthread_h
#endif