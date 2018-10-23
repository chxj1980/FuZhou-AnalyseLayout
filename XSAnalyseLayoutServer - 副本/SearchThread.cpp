#include "StdAfx.h"
#include "SearchThread.h"

int CSearchThread::m_nThreadNum = 1;

CSearchThread::CSearchThread()
{
    m_pZeromqManage = NULL;
    m_nThreadID = m_nThreadNum++;
    m_pAllFeatureNodeInfo = NULL;
}


CSearchThread::~CSearchThread()
{
}
bool CSearchThread::Init(char * pServerIP, int nServerPushPort, int nServerPullPort, LPALLFEATURENODEINFO pAllFeatureNodeInfo)
{
    m_pAllFeatureNodeInfo = pAllFeatureNodeInfo;
    if (NULL == m_pZeromqManage)
    {
        m_pZeromqManage = new CZeromqManage;
        if (!m_pZeromqManage->InitPull(NULL, 0, pServerIP, nServerPushPort, ZeromqSubMsg, this, 1))
        {
            return false;
        }
        if (!m_pZeromqManage->InitPush(NULL, 0, pServerIP, nServerPullPort))
        {
            return false;
        }
    }
    return true;
}
bool CSearchThread::UnInit()
{
    if (NULL != m_pZeromqManage)
    {
        m_pZeromqManage->UnInitPull();
        delete m_pZeromqManage;
        m_pZeromqManage = NULL;
    }
    return true;
}
void CSearchThread::ZeromqSubMsg(LPSUBMESSAGE pSubMessage, void * pUser)
{
    CSearchThread * pThis = (CSearchThread *)pUser;
    printf("�߳�[%d]�յ���������[%s]\n", pThis->m_nThreadID, pSubMessage->pHead);
    pThis->ParseZeromqJson(pSubMessage);
}
//Zeromq�ص���Ϣ��������
bool CSearchThread::ParseZeromqJson(LPSUBMESSAGE pSubMessage)
{
    rapidjson::Document document;
    if (string(pSubMessage->pSubJsonValue) != "")
    {
        document.Parse(pSubMessage->pSubJsonValue);
        if (document.HasParseError())
        {
            printf("***Warning: Parse Json Format Failed[%s].\n", pSubMessage->pSubJsonValue);
            return false;
        }
    }
    if (document.HasMember(JSONFACDUUID) && document[JSONFACDUUID].IsString() && strlen(document[JSONFACDUUID].GetString()) < MAXLEN        &&
        document.HasMember(JSONFEATURE)  && document[JSONFEATURE].IsString()  && strlen(document[JSONFEATURE].GetString()) < FEATURELEN     &&
                                                                                 strlen(document[JSONFEATURE].GetString()) > FEATUREMIXLEN  &&
        document.HasMember(JSONDEVICEID) && document[JSONDEVICEID].IsString() && strlen(document[JSONDEVICEID].GetString()) < MAXLEN        &&
        document.HasMember(JSONTIME)     && document[JSONTIME].IsInt()                                                                      &&
        document.HasMember(JSONDRIVE)    && document[JSONDRIVE].IsString()    && strlen(document[JSONDRIVE].GetString()) == 1               &&
        document.HasMember(JSONSERVERIP) && document[JSONSERVERIP].IsString() && strlen(document[JSONSERVERIP].GetString()) < MAXIPLEN      &&
        document.HasMember(JSONFACERECT) && document[JSONFACERECT].IsString() && strlen(document[JSONFACERECT].GetString()) < MAXLEN        &&
        document.HasMember(JSONSCORE)    && document[JSONSCORE].IsInt()                                                                     &&
        document.HasMember(JSONADDRESS)  && document[JSONADDRESS].IsInt()
        )
    {
        strcpy(pSubMessage->pFaceUUID, document[JSONFACDUUID].GetString());

        strcpy(pSubMessage->pFeature, document[JSONFEATURE].GetString());
        //----��������ֵΪ����������
        int nDLen = 0;
        string sDFeature = ZBase64::Decode(pSubMessage->pFeature, strlen(pSubMessage->pFeature), nDLen);
        memcpy(pSubMessage->pFeature, sDFeature.c_str(), sDFeature.size());

        strcpy(pSubMessage->pDeviceID, document[JSONDEVICEID].GetString());
        pSubMessage->nImageTime = document[JSONTIME].GetInt();
        strcpy(pSubMessage->pDisk, document[JSONDRIVE].GetString());
        strcpy(pSubMessage->pImageIP, document[JSONSERVERIP].GetString());
        strcpy(pSubMessage->pFaceRect, document[JSONFACERECT].GetString());
        pSubMessage->nScore = document[JSONSCORE].GetInt();
        pSubMessage->nAddress = document[JSONADDRESS].GetInt();

        VerifyFeatureAction(pSubMessage);
    }
    return true;
}

void CSearchThread::VerifyFeatureAction(LPSUBMESSAGE pSubMessage)
{
    rapidjson::Document document;
    document.SetObject();
    rapidjson::Document::AllocatorType&allocator = document.GetAllocator();
    rapidjson::Value array(rapidjson::kArrayType);

    document.AddMember(JSONFACDUUID, rapidjson::StringRef(pSubMessage->pFaceUUID), allocator);
    document.AddMember(JSONDEVICEID, rapidjson::StringRef(pSubMessage->pDeviceID), allocator);
    document.AddMember(JSONTIME, pSubMessage->nImageTime, allocator);
    document.AddMember(JSONDRIVE, rapidjson::StringRef(pSubMessage->pDisk), allocator);
    document.AddMember(JSONSERVERIP, rapidjson::StringRef(pSubMessage->pImageIP), allocator);
    document.AddMember(JSONFACERECT, rapidjson::StringRef(pSubMessage->pFaceRect), allocator);

    int nSearchNodeNum = 0;
    int nHitCount = 0;

    MAPSINGLEFEATURENODE::iterator it = m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.find(pSubMessage->pHead);
    if (it == m_pAllFeatureNodeInfo->mapAllFeatureNodeInfo.end())
    {
        printf("***Warning: ���ؿ�[%s]δ�ҵ�!", pSubMessage->pHead);
        return;
    }
    MAPKEYLIBINFO::iterator itKeyLib = it->second->mapSingleFeatureNode.find(pSubMessage->nAddress);
    if (itKeyLib == it->second->mapSingleFeatureNode.end())
    {
        printf("***Warning: ���ؿ�[%s]��δ�ҵ���ַ[%d]!", pSubMessage->pHead, pSubMessage->nAddress);
        return;
    }

    LPKEYLIBINFO pKeyLib = itKeyLib->second;
    LPKEYLIBFEATUREDATA pFeatureNode = pKeyLib->pHeadFeature;
    while (NULL != pFeatureNode)
    {
        nSearchNodeNum++;
        int nScore = VerifyFeature((unsigned char*)pSubMessage->pFeature,
            (unsigned char*)pFeatureNode->pFeature, pFeatureNode->nFeatureLen);
        if (nScore == VerifyFailed)
        {
            break;
        }
        else
        {
            //printf("%d ", nScore);
            if (nScore >= pSubMessage->nScore)
            {
                nHitCount++;

                rapidjson::Value object(rapidjson::kObjectType);
                object.AddMember(JSONLAYOUTFACDUUID, rapidjson::StringRef(pFeatureNode->pFaceUUID), allocator);
                object.AddMember(JSONSCORE, nScore, allocator);
                object.AddMember(JSONDLIBID, rapidjson::StringRef(pSubMessage->pHead), allocator);

                array.PushBack(object, allocator);
                printf("��������[%s].\n", pFeatureNode->pFaceUUID);
            }
        }
        pFeatureNode = pFeatureNode->pNext;
    }

    if (nHitCount > 0)  //��������, �򷵻�������Ϣ, ���򲻷����κ���Ϣ
    {
        document.AddMember("data", array, allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        pSubMessage->sPubJsonValue = buffer.GetString();

        char pTemp[MAXLEN] = { 0 };
        strcpy(pTemp, pSubMessage->pSource);
        strcpy(pSubMessage->pSource, pSubMessage->pHead);
        strcpy(pSubMessage->pHead, pTemp);
        SendResponseMsg(pSubMessage);
    }
    else
    {
        printf("��������δ����...\n");
    }
    return;
}
//����ֵ�ȶ�
int CSearchThread::VerifyFeature(unsigned char * pf1, unsigned char * pf2, int nLen)
{                       
    float dbScore = 0.0;
    if (FRSResult_Ok == FRSFeatureMatch(pf1, pf2, dbScore))
    {
    }
    else
    {
        printf("***Warning: FRSFeatureMatch Failed, Stop Search!\n");
        return VerifyFailed;
    }
    //printf("Verify Score : %lf,  %d.\n", dbScore, int(dbScore * 100));
    return int(dbScore * 100);
}
void CSearchThread::SendResponseMsg(LPSUBMESSAGE pSubMessage)
{
    m_pZeromqManage->PushMessage(pSubMessage);
    return;
}