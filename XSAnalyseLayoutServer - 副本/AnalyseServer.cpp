#include "stdafx.h"

#include "AnalyseServer.h"
#include "stdio.h"
#include "time.h"

CLogRecorder g_LogRecorder;
CAnalyseServer::CAnalyseServer()
{
    m_pZeromqManage = NULL;
    m_nServerType = 3;
    m_pAllFeatureNodeInfo = NULL;
}
CAnalyseServer::~CAnalyseServer()
{

}
//��ʼ����
bool CAnalyseServer::StartAnalyseServer()
{
    if(!Init())
    {
        printf("***Warning: StartAnalyseServer::Init Failed.\n");
        return false;
    }

    Sleep(1000);
    printf("******************AnalyseLayoutServer Start******************\n\n");


    unsigned char pIn;
    while(true)
    {
        printf("\nInput: \n"
                "a: Show All LibInfo\n"
                "e: quit Service\n"
                );
        cin >> pIn;
        switch(pIn)
        {
        case 'a':
            {
                printf("\n==============================\n");
                printf("ServerID: %s\n", m_ConfigRead.m_sServerID.c_str());
                printf("Lib Info: \n");
                int nSeq = 1;
                for (map<string, CLibInfo *>::iterator it = m_mapLibInfo.begin();
                    it != m_mapLibInfo.end(); it++)
                {
                    printf("  [%03d]  %s  Input Number: %d\n", nSeq++, it->first.c_str(), it->second->GetTotalNum());
                }
               
                printf("\n==============================\n");
                break;
            }
        case 'e':
            {
                StopAnalyseServer();
                return true;
            }
        }

		Sleep(1000);
    }
    

    return true;
}
//ֹͣ����
bool CAnalyseServer::StopAnalyseServer()
{
    if (NULL != m_pZeromqManage)
    {
        m_pZeromqManage->UnInitSub();
        m_pZeromqManage->UnInitPub();
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }
    while(m_mapLibInfo.size() > 0)
    {
        map<string, CLibInfo *>::iterator it = m_mapLibInfo.begin();
        it->second->Stop();
        delete it->second;
        m_mapLibInfo.erase(it);
    }
    while (m_listSearchThread.size() > 0)
    {
        m_listSearchThread.front()->UnInit();
        delete m_listSearchThread.front();
        m_listSearchThread.pop_front();
    }
    if (NULL != m_pAllFeatureNodeInfo)
    {
        m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.clear();
        delete m_pAllFeatureNodeInfo;
        m_pAllFeatureNodeInfo = NULL;
    }
    mysql_close(&m_mysql);
    DeleteCriticalSection(&m_cs);
    printf("************************Stop Analyse Server************************\n");
    return true;
}
//��ʼ������
bool CAnalyseServer::Init()
{
    DWORD nBufferLenth = MAX_PATH;
    char szBuffer[MAX_PATH] = { 0 };
    DWORD dwRet = GetModuleFileNameA(NULL, szBuffer, nBufferLenth);
    char *sPath = strrchr(szBuffer, '\\');
    memset(sPath, 0, strlen(sPath));
    string sConfigPath = szBuffer;
    sConfigPath += "/Config/XSAnalyseServer_x64_config.properties";
#ifdef _DEBUG
    sConfigPath = "./Config/XSAnalyseServer_x64_config.properties";
#endif
    g_LogRecorder.InitLogger(sConfigPath.c_str(), "XSAnalyseServer_x64Logger", "XSAnalyseServer_x64");

    //��ʼ���ȶԽӿ�
    printf("��ʼ��FRSLib...\n");
    int nRet = FRSInitialize();
    if(FRSResult_Ok != nRet)
    {
        printf("****Error: FRSInitialize Failed, ErrorCode: %d.\n", nRet);
        return false;
    }

    //��ȡ�����ļ�
    if(!m_ConfigRead.ReadConfig())
    {
        printf("****Error: Read Config File Failed.\n");
        return false;
    }

    if (string(m_ConfigRead.m_sServerID, 10, 3) == "253")
    {
        m_nServerType = 3;  //�������ط���
    }
    else
    {
        printf("����ID���ʹ���[%s].\n", m_ConfigRead.m_sServerID.c_str());
        return false;
    }
    //��ʼ���ٽ���
    InitializeCriticalSection(&m_cs);  

    m_pAllFeatureNodeInfo = new ALLFEATURENODEINFO;

    mysql_library_init(NULL,0,0);
    if(!ConnectDB())
    {
        printf("***Warning: connect MySQL Failed.\n");
        return false;
    }

    //��ʼ��Zeromq
    if(!InitZeromq())
    {
        return false;
    }

    Sleep(1000 * 3);
    if (!InitSearchThread())
    {
        return false;
    }

    return true;
}
//���ӱ������ݿ�
bool CAnalyseServer::ConnectDB()
{
    mysql_init(&m_mysql);
    if(!mysql_real_connect(&m_mysql, m_ConfigRead.m_sDBIP.c_str(), m_ConfigRead.m_sDBUser.c_str(), 
        m_ConfigRead.m_sDBPd.c_str(), m_ConfigRead.m_sDBName.c_str(), m_ConfigRead.m_nDBPort, NULL, 0))
    {
        const char * pErrorMsg = mysql_error(&m_mysql);
        printf("%s\n", pErrorMsg);
        printf("***Warning: CAnalyseServer::mysql_real_connect Failed, Please Check MySQL Service is start!\n");
        printf("DBInfo: %s:%d:%s:%s:%s\n",  m_ConfigRead.m_sDBIP.c_str(), m_ConfigRead.m_nDBPort, 
            m_ConfigRead.m_sDBName.c_str(), m_ConfigRead.m_sDBUser.c_str(), m_ConfigRead.m_sDBPd.c_str());
        return false;
    }
    else
    {
        printf("CAnalyseServer::Connect MySQL Success!\n");
    }
    
    if (!GetKeyLibInfo())
    {
        printf("***Warning: CAnalyseServer::GetKeyLibInfo Failed.\n");
        return false;
    }

    return true;
}
//��ȡ��ǰ���������ص����Ϣ
bool CAnalyseServer::GetKeyLibInfo()
{
    char pSQL[SQLMAXLEN] = { 0 };
    sprintf_s(pSQL, sizeof(pSQL),
        "select libname from %s where type = 3 order by libname", LIBINFOTABLE);
        //"select libname from %s where type = 2 order by libname", "checklibinfo");
    int nRet = mysql_query(&m_mysql, pSQL);
    if (nRet == 1)
    {
        printf("Excu SQL Failed, SQL:\n%s\n", pSQL);
        return false;
    }
    MYSQL_RES *result = NULL;
    result = mysql_store_result(&m_mysql);
    if (NULL == result)
    {
        printf("Excu SQL Failed, SQL:\n%s\n", pSQL);
        return false;
    }
    int nRowCount = mysql_num_rows(result);
    if (nRowCount > 0)
    {
        MYSQL_ROW row = NULL;
        row = mysql_fetch_row(result);
        while (NULL != row)
        {
            LPDEVICEINFO pDeviceInfo = new DEVICEINFO;
            pDeviceInfo->nServerType = m_nServerType;
            pDeviceInfo->sLibID = row[0];
            pDeviceInfo->nMaxCount = m_ConfigRead.m_nMaxCount;
            pDeviceInfo->sDBIP = m_ConfigRead.m_sDBIP;
            pDeviceInfo->nDBPort = m_ConfigRead.m_nDBPort;
            pDeviceInfo->sDBUser = m_ConfigRead.m_sDBUser;
            pDeviceInfo->sDBPassword = m_ConfigRead.m_sDBPd;
            pDeviceInfo->sDBName = m_ConfigRead.m_sDBName;
            pDeviceInfo->sPubServerIP = m_ConfigRead.m_sPubServerIP;
            pDeviceInfo->nPubServerPort = m_ConfigRead.m_nPubServerPort;
            pDeviceInfo->nSubServerPort = m_ConfigRead.m_nSubServerPort;

            LPSINGLEFEATURENODE pSingleFeatureNode = new SINGLEFEATURENODE;
            m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.insert(make_pair(pDeviceInfo->sLibID, pSingleFeatureNode));

            CLibInfo * pLibInfo = new CLibInfo;
            if (pLibInfo->Start(pDeviceInfo, pSingleFeatureNode))
            {
                m_mapLibInfo.insert(make_pair(pDeviceInfo->sLibID, pLibInfo));
            }
            else
            {
                printf("***Warning: Checkpoint[%s] Start Failed.\n", pDeviceInfo->sLibID.c_str());
            }
            row = mysql_fetch_row(result);
        }

    }
    mysql_free_result(result);
    return true;
}
//��ʼ��Zeromq
bool CAnalyseServer::InitZeromq()
{
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
    }
    if (!m_pZeromqManage->InitSub(NULL, 0, (char*)m_ConfigRead.m_sPubServerIP.c_str(), m_ConfigRead.m_nPubServerPort, ZeromqSubMsg, this, 1))
    {
        printf("****Error: ����[%s:%d:%s]ʧ��!", (char*)m_ConfigRead.m_sPubServerIP.c_str(), m_ConfigRead.m_nPubServerPort, m_ConfigRead.m_sServerID.c_str());
        return false;
    }
    m_pZeromqManage->AddSubMessage((char *)m_ConfigRead.m_sServerID.c_str());
    if(!m_pZeromqManage->InitPub(NULL, 0, (char*)m_ConfigRead.m_sPubServerIP.c_str(), m_ConfigRead.m_nSubServerPort))
    {
        printf("****Error: ��ʼ������ʧ��!");
        return false;
    }

    return true;
}
bool CAnalyseServer::InitSearchThread()
{
    SYSTEM_INFO SystemInfo;         //ϵͳ��Ϣ, ������ȡCpu����
    GetSystemInfo(&SystemInfo);
    for (DWORD i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
    {
        CSearchThread * pSearchThread = new CSearchThread;
        if (pSearchThread->Init((char *)m_ConfigRead.m_sLayoutPushIP.c_str(), 
                                m_ConfigRead.m_nLayoutPushPort, m_ConfigRead.m_nLayoutPullPort, m_pAllFeatureNodeInfo))
        {
            m_listSearchThread.push_back(pSearchThread);
        }
    }
    printf("--����[%d]�����ȶ��߳�...\n", SystemInfo.dwNumberOfProcessors);

    return true;
}
//Zeromq��Ϣ�ص�
void CAnalyseServer::ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CAnalyseServer * pThis = (CAnalyseServer *)pUser;
    pThis->ParseZeromqJson(pSubMessage);
}
//Zeromq�ص���Ϣ��������
bool CAnalyseServer::ParseZeromqJson(LPSUBMESSAGE pSubMessage)
{
    int nRet = 0;
    string sCommand(pSubMessage->pOperationType);
    rapidjson::Document document;
    if (string(pSubMessage->pSubJsonValue) != "")
    {
        document.Parse(pSubMessage->pSubJsonValue);
        if (document.HasParseError())
        {
            printf("***Warning: Parse Json Format Failed[%s].\n", pSubMessage->pSubJsonValue);
            SendResponseMsg(pSubMessage, JsonFormatError);
            return false;
        }
    }
    if(sCommand == COMMANDADDLIB)        //�����ص��
    {
        if (document.HasMember(JSONDLIBID) && document[JSONDLIBID].IsString())
        {
            pSubMessage->nTaskType = ADDLIB;
            string sDeviceID = document[JSONDLIBID].GetString();
            map<string, CLibInfo *>::iterator it = m_mapLibInfo.find(sDeviceID);
            if (it == m_mapLibInfo.end())
            {
                LPDEVICEINFO pDeviceInfo = new DEVICEINFO;
                pDeviceInfo->nServerType = m_nServerType;
                pDeviceInfo->sLibID = sDeviceID;
                pDeviceInfo->nMaxCount = m_ConfigRead.m_nMaxCount;
                pDeviceInfo->sDBIP = m_ConfigRead.m_sDBIP;
                pDeviceInfo->nDBPort = m_ConfigRead.m_nDBPort;
                pDeviceInfo->sDBUser = m_ConfigRead.m_sDBUser;
                pDeviceInfo->sDBPassword = m_ConfigRead.m_sDBPd;
                pDeviceInfo->sDBName = m_ConfigRead.m_sDBName;
                pDeviceInfo->sPubServerIP = m_ConfigRead.m_sPubServerIP;
                pDeviceInfo->nPubServerPort = m_ConfigRead.m_nPubServerPort;
                pDeviceInfo->nSubServerPort = m_ConfigRead.m_nSubServerPort;

                LPSINGLEFEATURENODE pSingleFeatureNode = new SINGLEFEATURENODE;
                m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.insert(make_pair(pDeviceInfo->sLibID, pSingleFeatureNode));

                CLibInfo * pLibInfo = new CLibInfo;
                if (pLibInfo->Start(pDeviceInfo, pSingleFeatureNode))
                {
                    if (!AddCheckpointToDB((char*)pDeviceInfo->sLibID.c_str()))
                    {
                        delete pLibInfo;
                        pLibInfo = NULL;
                        delete pDeviceInfo;
                        pDeviceInfo = NULL;
                        nRet = MysqlQueryFailed;
                    }
                    else
                    {
                        m_mapLibInfo.insert(make_pair(pDeviceInfo->sLibID, pLibInfo));
                    }
                }
                else
                {
                    printf("***Warning: Checkpoint[%s] Start Failed.\n", pDeviceInfo->sLibID.c_str());
                    delete pLibInfo;
                    pLibInfo = NULL;
                    delete pDeviceInfo;
                    pDeviceInfo = NULL;
                    nRet = CheckpointInitFailed;
                }
            }
            else
            {
                printf("***Warning: Add Checkpoint[%s] Aleady Exists!\n", sDeviceID.c_str());
                nRet = LibAleadyExist;
            }

            //��Ӧ��Ϣ�õ�
            strcpy(pSubMessage->pDeviceID, sDeviceID.c_str());
        }
        else
        {
            nRet = JsonFormatError;
        }
    }
    else if (sCommand == COMMANDDELLIB)  //ɾ���ص��
    {
        if (document.HasMember(JSONDLIBID) && document[JSONDLIBID].IsString())
        {
            pSubMessage->nTaskType = DELLIB;
            string sDeviceID = document[JSONDLIBID].GetString();
            map<string, CLibInfo *>::iterator it = m_mapLibInfo.find(sDeviceID);
            if (it == m_mapLibInfo.end())
            {
                printf("***Warning: Del Checkpoint[%s] Not Exist!\n", sDeviceID.c_str());
                nRet = LibNotExist;
            }
            else
            {
                if (!AddCheckpointToDB((char*)sDeviceID.c_str(), false))
                {
                    nRet = MysqlQueryFailed;
                }
                else
                {
                    it->second->Stop();
                    delete it->second;
                    m_mapLibInfo.erase(it);

                    m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.erase(sDeviceID);
                }
            }
            //��Ӧ��Ϣ�õ�
            strcpy(pSubMessage->pDeviceID, sDeviceID.c_str());
        }
        else
        {
            nRet = JsonFormatError;
        }
    }
    else
    {
        nRet = CommandNotFound;
    }

    strcpy(pSubMessage->pHead, pSubMessage->pSource);
    strcpy(pSubMessage->pSource, m_ConfigRead.m_sServerID.c_str());
    SendResponseMsg(pSubMessage, nRet);

    return true;
}
bool CAnalyseServer::AddCheckpointToDB(char * pDeviceID, bool bAdd)
{
    bool bRet = true;
    EnterCriticalSection(&m_cs);
    char pSQL[SQLMAXLEN] = { 0 };
    if (!bAdd)
    {
        sprintf(pSQL, "delete from %s where libname = '%s'", LIBINFOTABLE, pDeviceID);
    }
    else
    {
        sprintf(pSQL, "insert into %s(libname, type) values('%s', 3)", LIBINFOTABLE, pDeviceID);

    }

    if (bRet)
    {
        int nRet = mysql_query(&m_mysql, pSQL);
        if (nRet == 1)
        {
            const char * pErrorMsg = mysql_error(&m_mysql);
            printf("Excu SQL Failed[\"%s\"], SQL:\n%s\n", pErrorMsg, pSQL);
            bRet = false;
        }
    }
    
    LeaveCriticalSection(&m_cs);
    return bRet;
}
//��Ӧ����
void CAnalyseServer::SendResponseMsg(LPSUBMESSAGE pSubMessage, int nEvent)
{
    char pErrorMsg[64] = {0};
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);
    if(nEvent == 0)
    {
        document.AddMember("result", rapidjson::StringRef("success"), allocator);
        document.AddMember(JSONDLIBID, rapidjson::StringRef(pSubMessage->pDeviceID), allocator);
    }
    else
    {
        GetErrorMsg((ErrorCode)nEvent, pErrorMsg);
        document.AddMember("result", rapidjson::StringRef("error"), allocator);
        if (pSubMessage->nTaskType == ADDLIB || pSubMessage->nTaskType == DELLIB)
        {
            document.AddMember(JSONDLIBID, rapidjson::StringRef(pSubMessage->pDeviceID), allocator);
        }
        document.AddMember("errorcode", nEvent, allocator);
        document.AddMember("errorMessage", rapidjson::StringRef(pErrorMsg), allocator);
    }
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    pSubMessage->sPubJsonValue = buffer.GetString();
    //������Ӧ��Ϣ
    m_pZeromqManage->PubMessage(pSubMessage);
    printf("Rep Client json:\n%s\n==============================\n", pSubMessage->sPubJsonValue.c_str());
    return;
}
//��ȡ������˵��
int CAnalyseServer::GetErrorMsg(ErrorCode nError, char * pMsg)
{
    int nRet = 0;
    switch(nError)
    {
    case ServerNotInit:
        strcpy(pMsg, "SERVER_NOT_INIT");
        break;
    case DBAleadyExist:
        strcpy(pMsg, "DB_ALEADY_EXIST");
        break;
    case DBNotExist:
        strcpy(pMsg, "DB_NOT_EXIST");
        break;
    case FaceUUIDAleadyExist:
        strcpy(pMsg, "FACEUUID_ALEADY_EXIST");
        break;
    case FaceUUIDNotExist:
        strcpy(pMsg, "FACEUUID_NOT_EXIST");
        break;
    case ParamIllegal:
        strcpy(pMsg, "PARAM_ILLEGAL");
        break;
    case NewFailed:
        strcpy(pMsg, "NEW_FAILED");
        break;
    case JsonFormatError:
        strcpy(pMsg, "JSON_FORMAT_ERROR");
        break;
    case CommandNotFound:
        strcpy(pMsg, "COMMAND_NOT_FOUND");
        break;
    case HttpMsgUpperLimit:
        strcpy(pMsg, "HTTPMSG_UPPERLIMIT");
        break;
    case PthreadMutexInitFailed:
        strcpy(pMsg, "PTHREAD_MUTEX_INIT_FAILED");
        break;
    case FeatureNumOverMax:
        strcpy(pMsg, "FEATURE_NUM_OVER_MAX");
        break;
    case JsonParamIllegal:
        strcpy(pMsg, "JSON_PARAM_ILLEGAL");
        break;
    case MysqlQueryFailed:
        strcpy(pMsg, "MYSQL_QUERY_FAILED");
        break;
    case ParamLenOverMax:
        strcpy(pMsg, "PARAM_LEN_OVER_MAX");
        break;
    case LibAleadyExist:
        strcpy(pMsg, "LIB_ALEADY_EXIST");
        break;
    case LibNotExist:
        strcpy(pMsg, "LIB_NOT_EXIST");
        break;
    case CheckpointInitFailed:
        strcpy(pMsg, "CHECKPOINT_INIT_FAILED");
        break;
    case VerifyFailed:
        strcpy(pMsg, "VERIFY_FAILED");
        break;
    case HttpSocketBindFailed:
        strcpy(pMsg, "HTTP_SOCKET_BIND_FAILED");
        break;
    case CreateTableFailed:
        strcpy(pMsg, "CREATE_TABLE_FAILED");
        break;
    case SearchTimeWrong:
        strcpy(pMsg, "SEARCH_TIME_WRONG");
        break;
    case SearchNoDataOnTime:
        strcpy(pMsg, "SEARCH_NO_DATA_ON_TIME");
        break;
    case AddFeatureToCheckpointFailed:
        strcpy(pMsg, "ADD_FEATURE_TO_CHECKPOINT_FAILED");
        break;
    case SocketInitFailed:
        strcpy(pMsg, "SOCKET_INIT_FAILED");
        break;
    default:
        nRet = -1;
        break;
    }

    return nRet;
}
