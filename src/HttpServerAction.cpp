#include "stdafx.h"
#include "HttpServerAction.h"


extern CLogRecorder g_LogRecorder;
CHttpServerAction * g_pThis;
CHttpServerAction::CHttpServerAction()
{
    m_pTCPSocket = NULL;
    m_pHttpRequestCallback = NULL;
    g_pThis = this;
    m_hStopEvent = CreateEvent(NULL, true, false, NULL);
    InitializeCriticalSection(&m_cs);
    InitializeCriticalSection(&m_listcs);
}

CHttpServerAction::~CHttpServerAction()
{
    if (NULL != m_pTCPSocket)
    {
        delete m_pTCPSocket;
        m_pTCPSocket = NULL;
    }
    CloseHandle(m_hStopEvent);
    DeleteCriticalSection(&m_cs);
    DeleteCriticalSection(&m_listcs);
}

bool CHttpServerAction::StartHttpListen(char * pHttpIP, int nHttpPort, LPHTTPCLIENTREQUEST pHttpClientRequest, void * pUser)
{
    m_pHttpRequestCallback = pHttpClientRequest;
    m_pUser = pUser;

    m_pTCPSocket = new CTCPIOCPSocket;
    if (!m_pTCPSocket->InitSocket(pHttpIP, nHttpPort, MSGCALLBACK, true))
    {
        g_LogRecorder.WriteErrorLogEx(__FUNCTION__,
            "****Error: HTTP Socket[%s:%d]初始化失败!", pHttpIP, nHttpPort);
        return false;
    }
    else
    {
        g_LogRecorder.WriteDebugLogEx(__FUNCTION__, "开启HTTP服务[%s:%d].", pHttpIP, nHttpPort);
    }
    CreateThread(NULL, 0, ClientRequestCallbackThread, this, NULL, 0);     //http请求处理线程

    return true;
}
void CHttpServerAction::StopHttpServer()
{ 
    m_pTCPSocket->StopSocket();
    m_pHttpRequestCallback = NULL;
    SetEvent(m_hStopEvent);
    return;
}
bool CHttpServerAction::ResponseBody(SOCKET ClientSocket, string sRecvHttpMsg, string sResponseBody)
{
    HttpProtocol HttpInfo;
    HttpInfo.setRequestMethod(HL_HTTP_RESPONSE);
    HttpInfo.setRequestProperty("Access-Control-Allow-Origin", "*");
    HttpInfo.setRequestProperty("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, User-Agent");
    HttpInfo.setRequestProperty("Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
    HttpInfo.setRequestProperty("Allow", "*");
    HttpInfo.setRequestProperty("Content-Type", "text/plain; charset=utf-8");

    HttpInfo.SetHttpBody(sResponseBody);
    string sSendMsg = HttpInfo.GetHTTPMsg();
    m_pTCPSocket->SendData((char*)sSendMsg.c_str(), sSendMsg.size(), ClientSocket);

    return true;
}
void CALLBACK CHttpServerAction::MSGCALLBACK(SOCKET RemoteSocket, SOCKADDR_IN RemoteAddr, string sMsg, int nEvent)
{
    string sRemoteIP = inet_ntoa(RemoteAddr.sin_addr);
    int nRemotePort = ntohs(RemoteAddr.sin_port);
    switch (nEvent)
    {
    case 0:
        g_pThis->ParseMessage(RemoteSocket, RemoteAddr, sMsg);
        break;
    default:
        break;
    }
}
void CHttpServerAction::ParseMessage(SOCKET ClientSocket, SOCKADDR_IN RemoteAddr, string RecvBuf)
{
    EnterCriticalSection(&m_cs);
    std::map<SOCKET, LPHTTPMSG>::iterator it = m_mapClientMsg.find(ClientSocket);
    if (it == m_mapClientMsg.end())
    {
        size_t nPos = RecvBuf.find("Content-Length:");
        if (nPos == string::npos)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__, 
                "***Warning: 收到HTTP消息, 无Content-Length字段.\r\n%s", RecvBuf.c_str());
            LeaveCriticalSection(&g_pThis->m_cs);
            return;
        }
        else
        {
            nPos += strlen("Content-Length:");
            size_t nPos2 = RecvBuf.find("\r\n", nPos);
            if (nPos2 == string::npos)
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                    "***Warning: 收到HTTP消息, Content-Length字段后没找到换行回车符.\r\n%s", RecvBuf.c_str());
                LeaveCriticalSection(&g_pThis->m_cs);
                return;
            }

            string sBodyLen(RecvBuf, nPos, nPos2 - nPos);
            int nBodyLen = atoi(sBodyLen.c_str());

            nPos = RecvBuf.find("\r\n\r\n");
            if (nPos == string::npos)
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                    "***Warning: 收到HTTP消息, 未找到分界符.\r\n%s", RecvBuf.c_str());
                LeaveCriticalSection(&g_pThis->m_cs);
                return;
            }
            nPos += 4;
            string sHead(RecvBuf, 0, nPos);

            string sHttpBody(RecvBuf, nPos, RecvBuf.size() - nPos);
            int nCurBodyLen = RecvBuf.size() - nPos;
            if (nCurBodyLen == nBodyLen)
            {
                LPHTTPREQUEST pHttpRequest = new HTTPREQUEST;
                pHttpRequest->sHttpHead = sHead;
                pHttpRequest->sHttpBody = sHttpBody;
                pHttpRequest->ClientSocket = ClientSocket;
                EnterCriticalSection(&m_listcs);
                m_listHTTPRequest.push_back(pHttpRequest);
                LeaveCriticalSection(&m_listcs);

            }
            else
            {
                LPHTTPMSG pHttpMsg = new HTTPMSG;
                pHttpMsg->sHttpHead = sHead;
                pHttpMsg->nBodyLen = atoi(sBodyLen.c_str());
                if (RecvBuf.size() > nPos)
                {
                    pHttpMsg->sHttpBody = sHttpBody;
                    pHttpMsg->nCurBodyLen = nCurBodyLen;
                }
                m_mapClientMsg.insert(make_pair(ClientSocket, pHttpMsg));
            }
        }
    }
    else
    {
        it->second->sHttpBody += RecvBuf;
        it->second->nCurBodyLen += RecvBuf.size();
        if (it->second->nCurBodyLen >= it->second->nBodyLen)
        {
            LPHTTPREQUEST pHttpRequest = new HTTPREQUEST;
            pHttpRequest->sHttpHead = it->second->sHttpHead;
            pHttpRequest->sHttpBody = it->second->sHttpBody;
            pHttpRequest->ClientSocket = ClientSocket;
            EnterCriticalSection(&m_listcs);
            m_listHTTPRequest.push_back(pHttpRequest);
            LeaveCriticalSection(&m_listcs);

            delete it->second;
            m_mapClientMsg.erase(it);
        }
    }

    time_t tCurTime = time(&tCurTime);
    it = m_mapClientMsg.begin();
    while (it != m_mapClientMsg.end())
    {
        if (tCurTime - it->second->tRecvTime > HTTPMSGOVERTIME)
        {
            g_LogRecorder.WriteWarnLogEx(__FUNCTION__,
                "***Warning: 消息超时%d秒, 删除.\r\n%s", HTTPMSGOVERTIME, it->second->sHttpHead.c_str());
            delete it->second;
            it = m_mapClientMsg.erase(it);
        }
        else
        {
            it++;
        }
    }
    LeaveCriticalSection(&m_cs);
    return;
}
DWORD WINAPI CHttpServerAction::ClientRequestCallbackThread(LPVOID lParam)
{
    CHttpServerAction * pThis = (CHttpServerAction*)lParam;
    pThis->ClientRequestCallbackAction();
    return 0;
}
//http请求信息处理线程
void CHttpServerAction::ClientRequestCallbackAction()
{
    LPHTTPREQUEST pHttpRequest = NULL;
    while (WAIT_TIMEOUT == WaitForSingleObject(m_hStopEvent, THREADWAIT))
    {
        do
        {
            EnterCriticalSection(&m_listcs);
            if(m_listHTTPRequest.size() > 0)
            {
                pHttpRequest = m_listHTTPRequest.front();
                m_listHTTPRequest.pop_front();
            }
            else
            {
                pHttpRequest = NULL;
            }
            LeaveCriticalSection(&m_listcs);
            if(NULL == pHttpRequest)
            {
                continue;
            }
            bool bJsonFormat = false;    //Json串格式是否正确

#ifdef BATCHSTORELIBSERVER
            if (pHttpRequest->sHttpHead.find("Store/addpicture") != string::npos)
            {
                pHttpRequest->nOperatorType = 1;
            }
            else if (pHttpRequest->sHttpHead.find("Store/delpicture") != string::npos)
            {
                pHttpRequest->nOperatorType = 2;
            }
            else if (pHttpRequest->sHttpHead.find("Store/dellib") != string::npos)
            {
                pHttpRequest->nOperatorType = 5;
            }
            else
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "未找到操作类型\r\n%s", pHttpRequest->sHttpBody);

                delete pHttpRequest;
                continue;
            }

            rapidjson::Document document;
            document.Parse(pHttpRequest->sHttpBody.c_str());
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "解析Json串失败[%s]", pHttpRequest->sHttpBody.c_str());

                delete pHttpRequest;
                continue;
            }

            pHttpRequest->pStoreInfo = new STOREINFO;
            switch (pHttpRequest->nOperatorType)
            {
            case 1:
                {
                    if(document.HasMember("StoreLibID") && document["StoreLibID"].IsInt())
                    {
                        pHttpRequest->pStoreInfo->nStoreLibID = document["StoreLibID"].GetInt();
                        if(document.HasMember("Photo") && document["Photo"].IsArray() && document["Photo"].Size() > 0)
                        {
                            for(int i = 0; i < document["Photo"].Size(); i ++)
                            {
                                if(document["Photo"][i].HasMember("Face") && document["Photo"][i].HasMember("Name") && 
                                    document["Photo"][i]["Face"].IsString() && document["Photo"][i]["Name"].IsString())
                                {
                                    string sZBase64Face = document["Photo"][i]["Face"].GetString();
                                    string sFaceName = document["Photo"][i]["Name"].GetString();
                                    if(sFaceName == "" || sZBase64Face == "")
                                    {
                                        continue;
                                    }
                                    MAPZBASE64FACE::iterator itZBase64Face = pHttpRequest->pStoreInfo->mapZBase64FaceInfo.find(sFaceName);
                                    if(itZBase64Face == pHttpRequest->pStoreInfo->mapZBase64FaceInfo.end())
                                    {
                                        pHttpRequest->pStoreInfo->mapZBase64FaceInfo.insert(make_pair(sFaceName, sZBase64Face));
                                    }
                                    else
                                    {
                                        g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: 图片文件名[%s]己存在!", sFaceName.c_str());
                                    }
                                    bJsonFormat = true;
                                }
                                else
                                {
                                    bJsonFormat = false;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            case 2:
                {
                    if(document.HasMember("StoreLibID") && document["StoreLibID"].IsInt())
                    {
                        pHttpRequest->pStoreInfo->nStoreLibID = document["StoreLibID"].GetInt();
                        if(document.HasMember("StoreFace") && document["StoreFace"].IsArray() && document["StoreFace"].Size() > 0)
                        {
                            for (int i = 0; i < document["StoreFace"].Size(); i++)
                            {
                                if(document["StoreFace"][i].HasMember("FaceUUID"))
                                {
                                    string sFaceUUID = document["StoreFace"][i]["FaceUUID"].GetString();
                                    pHttpRequest->pStoreInfo->listFaceUUID.push_back(sFaceUUID);
                                    bJsonFormat = true;
                                }
                                else
                                {
                                    bJsonFormat = false;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            case 5:
                {
                    if(document.HasMember("StoreLibID") && document["StoreLibID"].IsInt())
                    {
                        pHttpRequest->pStoreInfo->nStoreLibID = document["StoreLibID"].GetInt();
                        bJsonFormat = true;
                    }
                }
            default:
                break;
            }
#endif

#ifdef LAYOUTSERVER

            if (pHttpRequest->sHttpHead.find("Layout/addpicture") != string::npos)
            {
                pHttpRequest->nOperatorType = 1;
            }
            else if (pHttpRequest->sHttpHead.find("Layout/delpicture") != string::npos)
            {
                pHttpRequest->nOperatorType = 2;
            }
            else if (pHttpRequest->sHttpHead.find("Layout/stoplib") != string::npos)
            {
                pHttpRequest->nOperatorType = 5;
            }
            else if (pHttpRequest->sHttpHead.find("Layout/addcheckpoint") != string::npos)
            {
                pHttpRequest->nOperatorType = 3;
            }
            else if (pHttpRequest->sHttpHead.find("Layout/delcheckpoint") != string::npos)
            {
                pHttpRequest->nOperatorType = 4;
            }
            else
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "未找到操作类型\r\n%s", pHttpRequest->sHttpBody);

                delete pHttpRequest;
                continue;
            }

            rapidjson::Document document;
            document.Parse(pHttpRequest->sHttpBody.c_str());
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "解析Json串失败[%s]", pHttpRequest->sHttpBody.c_str());

                delete pHttpRequest;
                continue;
            }

            pHttpRequest->pLayoutInfo = new LAYOUTINFO;
            switch (pHttpRequest->nOperatorType)
            {
            case 1: case 2:
            {
                if(document.HasMember("LayoutLibID") && document["LayoutLibID"].IsInt())
                {
                    pHttpRequest->pLayoutInfo->nLayoutLibID = document["LayoutLibID"].GetInt();
                    if(document.HasMember("LayoutFace") && document["LayoutFace"].IsArray() && document["LayoutFace"].Size() > 0)
                    {
                        for (int i = 0; i < document["LayoutFace"].Size(); i++)
                        {
                            if(document["LayoutFace"][i].HasMember("FaceUUID") && document["LayoutFace"][i]["FaceUUID"].IsString())
                            {
                                string sFaceUUID = document["LayoutFace"][i]["FaceUUID"].GetString();
                                pHttpRequest->pLayoutInfo->listFaceUUID.push_back(sFaceUUID);
                                bJsonFormat = true;
                            }
                            else
                            {
                                bJsonFormat = false;
                                break;
                            }
                        }
                    }
                }
                break;
            }
            case 3:
            {
                if(document.HasMember("LayoutLibID") && document["LayoutLibID"].IsInt())
                {
                    pHttpRequest->pLayoutInfo->nLayoutLibID = document["LayoutLibID"].GetInt();
                    if(document.HasMember("BeginTime") && document.HasMember("EndTime") &&
                        document["BeginTime"].IsString() && document["EndTime"].IsString())
                    {
                        string sBeginTime = document["BeginTime"].GetString();
                        string sEndTime = document["EndTime"].GetString();
                        sprintf_s(pHttpRequest->pLayoutInfo->pBeginTime, sizeof(pHttpRequest->pLayoutInfo->pBeginTime), sBeginTime.c_str());
                        sprintf_s(pHttpRequest->pLayoutInfo->pEndTime, sizeof(pHttpRequest->pLayoutInfo->pEndTime), sEndTime.c_str());
                        bJsonFormat = true;
                    }
                    if(document.HasMember("Checkpoint") && document["Checkpoint"].IsArray() && document["Checkpoint"].Size() > 0)
                    {
                        for (int i = 0; i < document["Checkpoint"].Size(); i++)
                        {
                            if(document["Checkpoint"][i].HasMember("DeviceID") && document["Checkpoint"][i].HasMember("Score") && 
                                document["Checkpoint"][i]["DeviceID"].IsString() && document["Checkpoint"][i]["Score"].IsInt())
                            {
                                string sCheckPoint = document["Checkpoint"][i]["DeviceID"].GetString();
                                unsigned int nScore = document["Checkpoint"][i]["Score"].GetInt();
                                pHttpRequest->pLayoutInfo->mapCheckpoint.insert(make_pair(sCheckPoint, nScore));
                                bJsonFormat = true;
                            }
                            else
                            {
                                bJsonFormat = false;
                                break;
                            }
                        }
                    }
                }
                break;
            }
            case 4:
            {
                if(document.HasMember("LayoutLibID") && document["LayoutLibID"].IsInt())
                {
                    pHttpRequest->pLayoutInfo->nLayoutLibID = document["LayoutLibID"].GetInt();
                    if(document.HasMember("Checkpoint") && document["Checkpoint"].IsArray() && document["Checkpoint"].Size() > 0)
                    {
                        for (int i = 0; i < document["Checkpoint"].Size(); i++)
                        {
                            if(document["Checkpoint"][i].HasMember("DeviceID") && document["Checkpoint"][i]["DeviceID"].IsString())
                            {
                                string sCheckPoint = document["Checkpoint"][i]["DeviceID"].GetString();
                                pHttpRequest->pLayoutInfo->mapCheckpoint.insert(make_pair(sCheckPoint, 0));
                                bJsonFormat = true;
                            }
                            else
                            {
                                bJsonFormat = false;
                            }
                        }
                    }
                }
                break;
            }
            case 5:
            {
                if(document.HasMember("LayoutLibID") && document["LayoutLibID"].IsInt())
                {
                    pHttpRequest->pLayoutInfo->nLayoutLibID = document["LayoutLibID"].GetInt();
                    bJsonFormat = true;
                }
            }
            default:
                break;
            }

#endif

#ifdef VERIFYSERVER       //对比服务
            if (pHttpRequest->sHttpHead.find("face/verify") != string::npos)
            {
                pHttpRequest->nOperatorType = 1;
            }
            else
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "未找到操作类型\r\n%s", pHttpRequest->sHttpBody);

                delete pHttpRequest;
                continue;
            }

            rapidjson::Document document;
            document.Parse(pHttpRequest->sHttpBody.c_str());
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "解析Json串失败[%s]", pHttpRequest->sHttpBody.c_str());

                delete pHttpRequest;
                continue;
            }

            switch (pHttpRequest->nOperatorType)
            {
            case 1:
                {
                    if(document.HasMember("imageone") && document["imageone"].IsString() && 
                        document.HasMember("imagetwo") && document["imagetwo"].IsArray() && 
                        document["imagetwo"].Size() > 0)
                    {
                        pHttpRequest->pVerifyInfo = new VERIFYINFO;
                        pHttpRequest->pVerifyInfo->sImageOne = document["imageone"].GetString();
                        if(pHttpRequest->pVerifyInfo->sImageOne  != "")
                        {
                            for(int i = 0; i < document["imagetwo"].Size(); i ++)
                            {
                                if(document["imagetwo"][i].HasMember("image") && document["imagetwo"][i]["image"].IsString() && 
                                    document["imagetwo"][i].HasMember("id")  && document["imagetwo"][i]["id"].IsInt())
                                {
                                    LPIMAGETWOINFO pImageTwoInfo = new IMAGETWOINFO;
                                    pImageTwoInfo->sImageTwo = document["imagetwo"][i]["image"].GetString();
                                    pImageTwoInfo->nImageID = document["imagetwo"][i]["id"].GetInt();
                                    if(pImageTwoInfo->sImageTwo == "")
                                    {
                                        bJsonFormat = false;
                                        break;
                                    }
                                    pHttpRequest->pVerifyInfo->listImageTwoInfo.push_back(pImageTwoInfo);
                                    bJsonFormat = true;
                                }
                                else
                                {
                                    bJsonFormat = false;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
            default:
                break;
            }

#endif

#ifdef COLLECTIONSERVER
            rapidjson::Document document;
            document.Parse(pHttpRequest->sHttpBody.c_str());
            if(document.HasParseError())
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "解析Json串失败[%s]", pHttpRequest->sHttpBody.c_str());
                delete pHttpRequest;
                continue;
            }

            LPDEVICEINFO pDeviceInfo = new DEVICEINFO;
            strcpy_s(pDeviceInfo->pDeviceID, sizeof(pDeviceInfo->pDeviceID), document["DeviceID"].GetString());
            strcpy_s(pDeviceInfo->pType, sizeof(pDeviceInfo->pType), document["type"].GetString());
            strcpy_s(pDeviceInfo->pName, sizeof(pDeviceInfo->pName), document["Name"].GetString());
            strcpy_s(pDeviceInfo->pIP, sizeof(pDeviceInfo->pIP), document["IP"].GetString());
            strcpy_s(pDeviceInfo->pPort, sizeof(pDeviceInfo->pPort), document["Port"].GetString());
            strcpy_s(pDeviceInfo->pChannel, sizeof(pDeviceInfo->pChannel), document["Channel"].GetString());
            strcpy_s(pDeviceInfo->pUserName, sizeof(pDeviceInfo->pUserName), document["UserName"].GetString());
            strcpy_s(pDeviceInfo->pPassword, sizeof(pDeviceInfo->pPassword), document["Password"].GetString());
            strcpy_s(pDeviceInfo->pEnableGate, sizeof(pDeviceInfo->pEnableGate), document["EnableGate"].GetString());

            pHttpRequest->pDeviceInfo = pDeviceInfo;
#endif

            if(!bJsonFormat)
            {
                g_LogRecorder.WriteWarnLogEx(__FUNCTION__, "***Warning: Json格式串错误!\r\n%s.", pHttpRequest->sHttpBody.c_str());
                char pHttpBody[] = "{\"result\":\"fail\", \"errormsg\":\"JSON_FORMAT_ERROR\"}";
                ResponseBody(pHttpRequest->ClientSocket, "", pHttpBody);
                delete pHttpRequest;
                continue;
            }

            m_pHttpRequestCallback(pHttpRequest, m_pUser);
        }while(pHttpRequest != NULL);
    }
    return;
}