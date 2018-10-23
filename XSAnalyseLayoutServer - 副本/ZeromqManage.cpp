#include "StdAfx.h"
#include "ZeromqManage.h"

CZeromqManage::CZeromqManage()
{
    m_pSubMsgCallback = NULL;
    m_pUser = NULL;
    m_bStopFlag = false;
    InitializeCriticalSection(&m_cs);

    m_pPubCtx = NULL;
    m_pPubSocket = NULL;

    m_pSubCtx = NULL;
    m_pSubSocket = NULL;

    m_pPullCtx = NULL;
    m_pPullSocket = NULL;

    m_pPushCtx = NULL;
    m_pPushSocket = NULL;

    m_ThreadSubMessageID = INVALID_HANDLE_VALUE;
    m_ThreadPullMessageID = INVALID_HANDLE_VALUE;
    for (int i = 0; i < THREADNUM; i++)
    {
        m_ThreadCallbackID[i] = INVALID_HANDLE_VALUE;
    }
}

CZeromqManage::~CZeromqManage()
{
    DeleteCriticalSection(&m_cs);
}
bool CZeromqManage::InitSub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort,
                            LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum)
{
    m_pSubMsgCallback = pSubMessageCallback;
    m_pUser = pUser;
    m_nThreadNum = nThreadNum;

    if (NULL == m_pSubCtx)
    {
        if (NULL == (m_pSubCtx = zmq_ctx_new()))
        {
            printf("****Error: zmq_ctx_new Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    
    if (NULL == m_pSubSocket)
    {
        if (NULL == (m_pSubSocket = zmq_socket(m_pSubCtx, ZMQ_SUB)))
        {
            zmq_ctx_destroy(m_pSubCtx);
            printf("****Error: zmq_socket Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pLocalIP && 0 != nLocalPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pLocalIP, nLocalPort);
        if (zmq_bind(m_pSubSocket, pAddress) < 0)
        {
            zmq_close(m_pSubSocket);
            zmq_ctx_destroy(m_pSubCtx);
            printf("****Error: zmq_bind[%s:%d] Failed[%s]!\n", pLocalIP, nLocalPort, zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pServerIP && 0 != nServerPort)
    {
        char pSubAddress[32] = { 0 };
        sprintf_s(pSubAddress, sizeof(pSubAddress), "tcp://%s:%d", pServerIP, nServerPort);
        if (zmq_connect(m_pSubSocket, pSubAddress) < 0)
        {
            zmq_close(m_pSubSocket);
            zmq_ctx_destroy(m_pSubCtx);
            printf("****Error: zmq_connect Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    
    for (int i = 0; i < m_nThreadNum * 20; i++)
    {
        LPSUBMESSAGE pSubMessage = new SUBMESSAGE;
        m_listSubMessageResource.push_back(pSubMessage);
    }
    if (m_ThreadSubMessageID == INVALID_HANDLE_VALUE)
    {
        m_ThreadSubMessageID = CreateThread(NULL, 0, SubMessageThread, this, NULL, 0);
    }
    for (int i = 0; i < m_nThreadNum; i++)
    {
        if (m_ThreadCallbackID[i] == INVALID_HANDLE_VALUE)
        {
            m_ThreadCallbackID[i] = CreateThread(NULL, 0, MessageCallbackThread, this, NULL, 0);
        }
    }

    return true;
}
void CZeromqManage::UnInitSub()
{
    m_bStopFlag = true;
    Sleep(100);
    if (NULL != m_pSubSocket)
    {
        zmq_close(m_pSubSocket);
        m_pSubSocket = NULL;
    }
    if (NULL != m_pSubCtx)
    {
        zmq_ctx_destroy(m_pSubCtx);
        m_pSubCtx = NULL;
    }
    if (INVALID_HANDLE_VALUE != m_ThreadSubMessageID)
    {
        WaitForSingleObject(m_ThreadSubMessageID, INFINITE);
        m_ThreadSubMessageID = INVALID_HANDLE_VALUE;
        printf("--Thread Sub Message End...\n");
    }
    for (int i = 0; i < m_nThreadNum; i++)
    {
        if (INVALID_HANDLE_VALUE != m_ThreadCallbackID[i])
        {
            WaitForSingleObject(m_ThreadCallbackID[i], INFINITE);
            m_ThreadCallbackID[i] = INVALID_HANDLE_VALUE;
            printf("--Thread[i] Callback Sub Message End...\n", i);
        }
    }
    EnterCriticalSection(&m_cs);
    while (m_listSubMessageResource.size() > 0)
    {
        delete m_listSubMessageResource.front();
        m_listSubMessageResource.pop_front();
    }
    LeaveCriticalSection(&m_cs);
    return;
}
bool CZeromqManage::AddSubMessage(char * pSubMessage)
{
    if (zmq_setsockopt(m_pSubSocket, ZMQ_SUBSCRIBE, pSubMessage, strlen(pSubMessage)) < 0)
    {
        printf("****Error: zmq_setsockopt Subscribe[%s] Failed[%s]!\n", pSubMessage, zmq_strerror(errno));
        return false;
    }
    else
    {
        printf("##订阅[%s]成功!\n", pSubMessage);
    }
    return true;
}
bool CZeromqManage::DelSubMessage(char * pSubMessage)
{
    if (zmq_setsockopt(m_pSubSocket, ZMQ_UNSUBSCRIBE, pSubMessage, strlen(pSubMessage)) < 0)
    {
        printf("****Error: zmq_setsockopt UnSubscribe[%s] Failed[%s]!\n", pSubMessage, zmq_strerror(errno));
        return false;
    }
    else
    {
        printf("##取消订阅[%s]成功!\n", pSubMessage);
    }
    return true;
}
DWORD WINAPI CZeromqManage::SubMessageThread(void * pParam)
{
    CZeromqManage * pThis = (CZeromqManage *)pParam;
    pThis->SubMessageAction();
    return 0;
}

void CZeromqManage::SubMessageAction()
{
    while (!m_bStopFlag)
    {
        LPSUBMESSAGE pSubMessage = GetFreeResource();
        if (NULL == pSubMessage)
        {
            printf("***Warning: 订阅线程当前无法获取到空闲资源!\n");
            Sleep(THREADWAITTIME * 100);
            continue;
        }
        int nRet = zmq_recv(m_pSubSocket, pSubMessage->pHead, sizeof(pSubMessage->pHead), 0);       //无消息时会阻塞
        if(nRet < 0)
        {
            printf("****error: %s\n", zmq_strerror(errno));
            FreeResource(pSubMessage);
        }
        else
        {
            bool bRecv = false;
            
            int nMore = 0;
            size_t nMoreSize = sizeof(nMore);
            zmq_getsockopt(m_pSubSocket, ZMQ_RCVMORE, &nMore, &nMoreSize);
            if (nMore)
            {
                zmq_recv(m_pSubSocket, pSubMessage->pOperationType, sizeof(pSubMessage->pOperationType), 0);
                zmq_setsockopt(m_pSubSocket, ZMQ_RCVMORE, &nMore, nMoreSize);
                if (nMore)
                {
                    zmq_recv(m_pSubSocket, pSubMessage->pSource, sizeof(pSubMessage->pSource), 0);
                    printf("\n------------Recv: \n");
                    printf("%s\n%s\n%s\n------------\n", pSubMessage->pHead, pSubMessage->pOperationType, pSubMessage->pSource);
                    zmq_setsockopt(m_pSubSocket, ZMQ_RCVMORE, &nMore, nMoreSize);
                    if (nMore)
                    {
                        zmq_recv(m_pSubSocket, pSubMessage->pSubJsonValue, sizeof(pSubMessage->pSubJsonValue), 0);
                        EnterCriticalSection(&m_cs);
                        m_listSubMessageCallback.push_back(pSubMessage);
                        LeaveCriticalSection(&m_cs);

                        bRecv = true;
                    }
                }
            }

            if(!bRecv)
            {
                printf("***Warning: Recv more Failed.\n");
                FreeResource(pSubMessage);
            }
        }
    }
    return;
}
DWORD WINAPI CZeromqManage::MessageCallbackThread(void * pParam)
{
    CZeromqManage * pThis = (CZeromqManage *)pParam;
    pThis->MessageCallbackAction();
    return 0;
}
void CZeromqManage::MessageCallbackAction()
{
    while (!m_bStopFlag)
    {
        LPSUBMESSAGE pSubMessage = NULL;
        EnterCriticalSection(&m_cs);
        if (m_listSubMessageCallback.size() > 0)
        {
            pSubMessage = m_listSubMessageCallback.front();
            m_listSubMessageCallback.pop_front();
        }
        LeaveCriticalSection(&m_cs);
        if (NULL != pSubMessage)
        {
            m_pSubMsgCallback(pSubMessage, m_pUser);
            FreeResource(pSubMessage);
        }

        Sleep(THREADWAITTIME);
    }
}
LPSUBMESSAGE CZeromqManage::GetFreeResource()
{
    LPSUBMESSAGE pSubMessage = NULL;
    EnterCriticalSection(&m_cs);
    if (m_listSubMessageResource.size() > 0)
    {
        pSubMessage = m_listSubMessageResource.front();
        m_listSubMessageResource.pop_front();
    }
    else
    {
        pSubMessage = new SUBMESSAGE;
        if (NULL == pSubMessage)
        {
            printf("****Error: new Failed!\n");
        }
    }
    LeaveCriticalSection(&m_cs);

    memset(pSubMessage, 0, sizeof(SUBMESSAGE));
    return pSubMessage;
}
void CZeromqManage::FreeResource(LPSUBMESSAGE pSubMessage)
{
    EnterCriticalSection(&m_cs);
    m_listSubMessageResource.push_back(pSubMessage);
    while (m_listSubMessageResource.size() > m_nThreadNum * 20)
    {
        delete m_listSubMessageResource.front();
        m_listSubMessageResource.pop_front();
    }
    LeaveCriticalSection(&m_cs);
}
bool CZeromqManage::InitPub(char * pLocalIP, int nLocalPort, char * pServerIP, int nServerPort)
{
    if (NULL == m_pPubCtx)
    {
        if (NULL == (m_pPubCtx = zmq_ctx_new()))
        {
            printf("****Error: zmq_ctx_new Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL == m_pPubSocket)
    {
        if (NULL == (m_pPubSocket = zmq_socket(m_pPubCtx, ZMQ_PUB)))
        {
            zmq_ctx_destroy(m_pPubCtx);
            printf("****Error: zmq_socket Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pLocalIP && 0 != nLocalPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pLocalIP, nLocalPort);
        if (zmq_bind(m_pPushSocket, pAddress) < 0)
        {
            zmq_close(m_pPubSocket);
            zmq_ctx_destroy(m_pPubCtx);
            printf("****Error: InitPub::zmq_bind[%s:%d] Failed[%s]!\n", pLocalIP, nLocalPort, zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pServerIP && 0 != nServerPort)
    {
        char pServerAddress[32] = { 0 };
        sprintf_s(pServerAddress, sizeof(pServerAddress), "tcp://%s:%d", pServerIP, nServerPort);
        if (zmq_connect(m_pPubSocket, pServerAddress) < 0)
        {
            zmq_close(m_pPubSocket);
            zmq_ctx_destroy(m_pPubCtx);
            printf("****Error: InitPub::zmq_connect[%s:%d] Failed[%s]!\n", pServerIP, nServerPort, zmq_strerror(errno));
            return false;
        }
    }
        
    return true;
}
void CZeromqManage::UnInitPub()
{
    if (NULL != m_pPubSocket)
    {
        zmq_close(m_pPubSocket);
        m_pPubSocket = NULL;
    }
    if (NULL != m_pPubCtx)
    {
        zmq_ctx_destroy(m_pPubCtx);
        m_pPubCtx = NULL;
    }
}
bool CZeromqManage::PubMessage(LPSUBMESSAGE pSubMessage)
{
    if (NULL != m_pPubSocket)
    {
        zmq_send(m_pPubSocket, pSubMessage->pHead, strlen(pSubMessage->pHead), ZMQ_SNDMORE);
        zmq_send(m_pPubSocket, pSubMessage->pOperationType, strlen(pSubMessage->pOperationType), ZMQ_SNDMORE);
        zmq_send(m_pPubSocket, pSubMessage->pSource, strlen(pSubMessage->pSource), ZMQ_SNDMORE);
        zmq_send(m_pPubSocket, pSubMessage->sPubJsonValue.c_str(), pSubMessage->sPubJsonValue.size(), 0);
    }
    else
    {
        printf("****Error: PubMessage Failed, NULL == m_pPubSocket!");
        return false;
    }
    
    return true;
}
bool CZeromqManage::PubMessage(char * pHead, char * pID)
{
    if (NULL != m_pPubSocket)
    {
        zmq_send(m_pPubSocket, pHead, strlen(pHead), ZMQ_SNDMORE);
        zmq_send(m_pPubSocket, pID, strlen(pID), 0);
    }
    else
    {
        printf("****Error: PubMessage Failed, NULL == m_pPubSocket!");
        return false;
    }

    return true;
}
bool CZeromqManage::InitPush(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort)
{
    if (NULL == m_pPushCtx)
    {
        if (NULL == (m_pPushCtx = zmq_ctx_new()))
        {
            printf("****Error: zmq_ctx_new Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL == m_pPushSocket)
    {
        if (NULL == (m_pPushSocket = zmq_socket(m_pPushCtx, ZMQ_PUSH)))
        {
            zmq_ctx_destroy(m_pSubCtx);
            printf("****Error: zmq_socket Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pLocalIP && 0 != nLocalPushPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pLocalIP, nLocalPushPort);
        if (zmq_bind(m_pPushSocket, pAddress) < 0)
        {
            zmq_close(m_pPushSocket);
            zmq_ctx_destroy(m_pPushCtx);
            printf("****Error: InitPush::zmq_bind Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    if (NULL != pServerIP && 0 != nServerPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pServerIP, nServerPort);
        if (zmq_connect(m_pPushSocket, pAddress) < 0)
        {
            zmq_close(m_pPushSocket);
            zmq_ctx_destroy(m_pPushCtx);
            printf("****Error: InitPush::zmq_connect[%s:%d] Failed[%s]!\n",pServerIP, nServerPort, zmq_strerror(errno));
            return false;
        }
    }
    
    return true;
}
void CZeromqManage::UnInitPush()
{
    if (NULL != m_pPushSocket)
    {
        zmq_close(m_pPushSocket);
        m_pPushSocket = NULL;
    }
    if (NULL != m_pPushCtx)
    {
        zmq_ctx_destroy(m_pPushCtx);
        m_pPushCtx = NULL;
    }
    
    return;
}
bool CZeromqManage::PushMessage(LPSUBMESSAGE pSubMessage)
{
    if (NULL != m_pPushSocket)
    {
        zmq_send(m_pPushSocket, pSubMessage->pHead, strlen(pSubMessage->pHead), ZMQ_SNDMORE);
        zmq_send(m_pPushSocket, pSubMessage->pOperationType, strlen(pSubMessage->pOperationType), ZMQ_SNDMORE);
        zmq_send(m_pPushSocket, pSubMessage->pSource, strlen(pSubMessage->pSource), ZMQ_SNDMORE);
        zmq_send(m_pPushSocket, pSubMessage->sPubJsonValue.c_str(), pSubMessage->sPubJsonValue.size(), 0);
    }
    else
    {
        printf("****Error: PushMessage Failed, NULL == m_pPushSocket!");
        return false;
    }

    return true;
}
bool CZeromqManage::InitPull(char * pLocalIP, int nLocalPushPort, char * pServerIP, int nServerPort,
                                LPSubMessageCallback pSubMessageCallback, void * pUser, int nThreadNum)
{
    m_pSubMsgCallback = pSubMessageCallback;
    m_pUser = pUser;
    m_nThreadNum = nThreadNum;

    if (NULL == m_pPullCtx)
    {
        if (NULL == (m_pPullCtx = zmq_ctx_new()))
        {
            printf("****Error: zmq_ctx_new Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }

    if (NULL == m_pPullSocket)
    {
        if (NULL == (m_pPullSocket = zmq_socket(m_pPullCtx, ZMQ_PULL)))
        {
            zmq_ctx_destroy(m_pPullCtx);
            printf("****Error: zmq_socket Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }

    if (NULL != pLocalIP && 0 != nLocalPushPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pLocalIP, nLocalPushPort);
        if (zmq_bind(m_pPullSocket, pAddress) < 0)
        {
            zmq_close(m_pPullSocket);
            zmq_ctx_destroy(m_pPullCtx);
            printf("****Error: zmq_bind[%s:%d] Failed[%s]!\n", pLocalIP, nLocalPushPort, zmq_strerror(errno));
            return false;
        }
    }

    if (NULL != pServerIP && 0 != nServerPort)
    {
        char pAddress[32] = { 0 };
        sprintf_s(pAddress, sizeof(pAddress), "tcp://%s:%d", pServerIP, nServerPort);
        if (zmq_connect(m_pPullSocket, pAddress) < 0)
        {
            zmq_close(m_pPullSocket);
            zmq_ctx_destroy(m_pPullCtx);
            printf("****Error: zmq_connect Failed[%s]!\n", zmq_strerror(errno));
            return false;
        }
    }
    

    if (0 == m_listSubMessageResource.size())
    {
        for (int i = 0; i < m_nThreadNum * 20; i++)
        {
            LPSUBMESSAGE pSubMessage = new SUBMESSAGE;
            m_listSubMessageResource.push_back(pSubMessage);
        }
    }
    
    if (INVALID_HANDLE_VALUE == m_ThreadPullMessageID)
    {
        m_ThreadPullMessageID = CreateThread(NULL, 0, PullMessageThread, this, NULL, 0);
    }
    for (int i = 0; i < m_nThreadNum; i++)
    {
        if (INVALID_HANDLE_VALUE == m_ThreadCallbackID[i])
        {
            m_ThreadCallbackID[i] = CreateThread(NULL, 0, MessageCallbackThread, this, NULL, 0);
        }
    }

    return true;
}
void CZeromqManage::UnInitPull()
{
    if (NULL != m_pPullSocket)
    {
        zmq_close(m_pPullSocket);
        m_pPullSocket = NULL;
    }
    if (NULL != m_pPullCtx)
    {
        zmq_ctx_destroy(m_pPullCtx);
        m_pPullCtx = NULL;
    }
    
    return;
}
DWORD WINAPI CZeromqManage::PullMessageThread(void * pParam)
{
    CZeromqManage * pThis = (CZeromqManage *)pParam;
    pThis->PullMessageAction();
    return 0;
}
void CZeromqManage::PullMessageAction()
{
    while (!m_bStopFlag)
    {
        LPSUBMESSAGE pSubMessage = GetFreeResource();
        if (NULL == pSubMessage)
        {
            printf("***Warning: 订阅线程当前无法获取到空闲资源!\n");
            Sleep(THREADWAITTIME * 100);
            continue;
        }
        int nRet = zmq_recv(m_pPullSocket, pSubMessage->pHead, sizeof(pSubMessage->pHead), 0);       //无消息时会阻塞
        if (nRet < 0)
        {
            printf("****error: %s\n", zmq_strerror(errno));
            FreeResource(pSubMessage);
        }
        else
        {
            bool bRecv = false;

            int nMore = 0;
            size_t nMoreSize = sizeof(nMore);
            zmq_getsockopt(m_pPullSocket, ZMQ_RCVMORE, &nMore, &nMoreSize);
            if (nMore)
            {
                zmq_recv(m_pPullSocket, pSubMessage->pOperationType, sizeof(pSubMessage->pOperationType), 0);
                zmq_setsockopt(m_pPullSocket, ZMQ_RCVMORE, &nMore, nMoreSize);
                if (nMore)
                {
                    zmq_recv(m_pPullSocket, pSubMessage->pSource, sizeof(pSubMessage->pSource), 0);
                    //printf("\n------------Recv: \n");
                    //printf("%s\n%s\n%s\n------------\n", pSubMessage->pHead, pSubMessage->pOperationType, pSubMessage->pSource);
                    zmq_setsockopt(m_pPullSocket, ZMQ_RCVMORE, &nMore, nMoreSize);
                    if (nMore)
                    {
                        zmq_recv(m_pPullSocket, pSubMessage->pSubJsonValue, sizeof(pSubMessage->pSubJsonValue), 0);
                        EnterCriticalSection(&m_cs);
                        m_listSubMessageCallback.push_back(pSubMessage);
                        LeaveCriticalSection(&m_cs);

                        bRecv = true;
                    }
                }
            }

            if (!bRecv)
            {
                printf("***Warning: Recv more Failed.\n");
                FreeResource(pSubMessage);
            }
        }
    }
    return;
}