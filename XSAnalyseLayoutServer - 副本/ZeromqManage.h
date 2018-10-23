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
    //发布消息
    bool InitPub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort);
    void UnInitPub();
    bool PubMessage(LPSUBMESSAGE pSubMessage);
    bool PubMessage(char * pHead, char * pID);
    //订阅消息
    bool InitSub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort,
                 LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum = THREADNUM);
    bool AddSubMessage(char * pSubMessage);
    bool DelSubMessage(char * pSubMessage);
    void UnInitSub();

    //绑定本地push端口
    bool InitPush(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort);
    void UnInitPush();
    bool PushMessage(LPSUBMESSAGE pSubMessage);
    //pull
    bool InitPull(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort, 
                    LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum = THREADNUM);
    void UnInitPull();
private:
    //订阅消息接收
    static DWORD WINAPI SubMessageThread(void * pParam);
    void SubMessageAction();
    //pull消息接收
    static DWORD WINAPI PullMessageThread(void * pParam);
    void PullMessageAction();


    static DWORD WINAPI MessageCallbackThread(void * pParam);
    void MessageCallbackAction();

    LPSUBMESSAGE GetFreeResource();
    void FreeResource(LPSUBMESSAGE pSubMessage);

private:
    LPSubMessageCallback m_pSubMsgCallback; //订阅消息回调
    void * m_pUser;                         //订阅消息回调返回初始化传入参数
    bool m_bStopFlag;                       //线程停止标志
    CRITICAL_SECTION m_cs;                  //临界区
    int m_nThreadNum;                       //回调线程数, 申请资源*20

    //发布Pub
    void * m_pPubCtx;
    void * m_pPubSocket;            //发布socket
    //订阅Sub
    void * m_pSubCtx;
    void * m_pSubSocket;            //订阅socket
    //推送Push
    void * m_pPushCtx;
    void * m_pPushSocket;            //推送socket
    //拉Pull
    void * m_pPullCtx;
    void * m_pPullSocket;            //拉socket

    HANDLE m_ThreadSubMessageID;    //订阅消息线程句柄
    HANDLE m_ThreadPullMessageID;    //pull消息线程句柄
    HANDLE m_ThreadCallbackID[THREADNUM];   //回调线程句柄
    LISTSUBMESSAGE m_listSubMessageResource;
    LISTSUBMESSAGE m_listSubMessageCallback;

    

    
};

