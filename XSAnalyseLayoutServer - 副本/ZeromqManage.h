#pragma once

#include "DataDefine.h"
#include "zmq.h"
#pragma comment( lib, "libzmq-v100-mt-gd-4_0_4.lib" )


class CZeromqManage
{
public:
    CZeromqManage();
    ~CZeromqManage();
public:
    //������Ϣ
    bool InitPub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort);
    void UnInitPub();
    bool PubMessage(LPSUBMESSAGE pSubMessage);
    bool PubMessage(char * pHead, char * pID);
    //������Ϣ
    bool InitSub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort,
                 LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum = THREADNUM);
    bool AddSubMessage(char * pSubMessage);
    bool DelSubMessage(char * pSubMessage);
    void UnInitSub();

    //�󶨱���push�˿�
    bool InitPush(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort);
    void UnInitPush();
    bool PushMessage(LPSUBMESSAGE pSubMessage);
    //pull
    bool InitPull(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort, 
                    LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum = THREADNUM);
    void UnInitPull();
private:
    //������Ϣ����
    static DWORD WINAPI SubMessageThread(void * pParam);
    void SubMessageAction();
    //pull��Ϣ����
    static DWORD WINAPI PullMessageThread(void * pParam);
    void PullMessageAction();


    static DWORD WINAPI MessageCallbackThread(void * pParam);
    void MessageCallbackAction();

    LPSUBMESSAGE GetFreeResource();
    void FreeResource(LPSUBMESSAGE pSubMessage);

private:
    LPSubMessageCallback m_pSubMsgCallback; //������Ϣ�ص�
    void * m_pUser;                         //������Ϣ�ص����س�ʼ���������
    bool m_bStopFlag;                       //�߳�ֹͣ��־
    CRITICAL_SECTION m_cs;                  //�ٽ���
    int m_nThreadNum;                       //�ص��߳���, ������Դ*20

    //����Pub
    void * m_pPubCtx;
    void * m_pPubSocket;            //����socket
    //����Sub
    void * m_pSubCtx;
    void * m_pSubSocket;            //����socket
    //����Push
    void * m_pPushCtx;
    void * m_pPushSocket;            //����socket
    //��Pull
    void * m_pPullCtx;
    void * m_pPullSocket;            //��socket

    HANDLE m_ThreadSubMessageID;    //������Ϣ�߳̾��
    HANDLE m_ThreadPullMessageID;    //pull��Ϣ�߳̾��
    HANDLE m_ThreadCallbackID[THREADNUM];   //�ص��߳̾��
    LISTSUBMESSAGE m_listSubMessageResource;
    LISTSUBMESSAGE m_listSubMessageCallback;

    

    
};

