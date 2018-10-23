#ifndef analyseserver_h

#include <map>
#include <string>
#include "ConfigRead.h"
#include "DataDefine.h"
#include "LibInfo.h"
#include "ZeromqManage.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include <mysql/mysql.h>
#include "SearchThread.h"
using namespace std;

#ifdef __WINDOWS__
using namespace NeoFaceV;
#else
#include <unistd.h>
#endif

class CAnalyseServer
{
public:
    CAnalyseServer();
    ~CAnalyseServer();
public:
    bool StartAnalyseServer();
    bool StopAnalyseServer();
private:
    bool Init();
    //����DB
    bool ConnectDB();    
    //��ȡ��ǰ���������ص����Ϣ
    bool GetKeyLibInfo();
    //��ʼ��Zeromq����
    bool InitZeromq();
    //��ʼ���ȶ��߳�(cpu�߳���)
    bool InitSearchThread();
    //���濨����Ϣ��DB, bAdd: true: ����, false: ɾ��
    bool AddCheckpointToDB(char * pDeviceID, bool bAdd = true);
private:
    //zeromq������Ϣ�ص�
    static void ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser);
private:
    //����Zeromq json
    bool ParseZeromqJson(LPSUBMESSAGE pSubMessage);
    //���ͻ�Ӧ��Ϣ
    void SendResponseMsg(LPSUBMESSAGE pSubMessage, int nEvent);
    //��ȡ������˵��
    int GetErrorMsg(ErrorCode nError, char * pMsg);
private:
    CConfigRead m_ConfigRead;   //�����ļ���ȡ
    int m_nServerType;          //��������
    MYSQL m_mysql;                      //����MySQL���ݿ�
    CZeromqManage * m_pZeromqManage;    //Zeromq����    
    map<string, CLibInfo *> m_mapLibInfo;
    LPALLFEATURENODEINFO m_pAllFeatureNodeInfo;

    list<CSearchThread * > m_listSearchThread;
    int m_nSearchThreadCount;
#ifdef __WINDOWS__
    CRITICAL_SECTION m_cs;
    HANDLE m_hStopEvent;
#else
    pthread_mutex_t m_cs;
#endif
};

#define analyseserver_h
#endif