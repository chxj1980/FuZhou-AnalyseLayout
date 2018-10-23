#include "stdafx.h"
#include "FeatureManage.h"

CFeatureManage::CFeatureManage(void)
{
    m_nFeatureIndex = 1;
}
CFeatureManage::~CFeatureManage(void)
{
}
int CFeatureManage::Init(char * pDeviceID, int nMaxCount, LPSINGLEFEATURENODE pSingleFeatureNode)
{
    //�������
    strcpy(m_pLibName, pDeviceID);
    m_nMaxCount = nMaxCount;
    m_pSingleFeatureNode = pSingleFeatureNode;

    return true;
}
bool CFeatureManage::UnInit()
{
#ifdef __WINDOWS__
    Sleep(100); //100 ms
#else
    usleep(1000 * 100);
#endif
    while (m_listKeyLibInfo.size() > 0)
    {
        delete m_listKeyLibInfo.front();
        m_listKeyLibInfo.pop_front();
    }
    
    return true;
}
int CFeatureManage::AddFeature(LPKEYLIBFEATUREDATA pFeatureData, bool bInit)
{
    int nRet = INVALIDERROR;
    //��������ֵ���
    LPKEYLIBINFO pKeyLibInfo = NULL;
    if (m_listKeyLibInfo.size() > 0 && m_listKeyLibInfo.back()->nNum < m_nMaxCount)
    {
        pKeyLibInfo = m_listKeyLibInfo.back();
    }
    else
    {
        pKeyLibInfo = new KEYLIBINFO;
        m_listKeyLibInfo.push_back(pKeyLibInfo);
        m_pSingleFeatureNode->mapSingleFeatureNode.insert(make_pair(m_nFeatureIndex++, pKeyLibInfo));
    }

    //MAPKEYLIBFEATUREDATA::iterator itFeature = pKeyLibInfo->mapFeature.find(pFeatureData->pFaceUUID);
    //if (itFeature != pKeyLibInfo->mapFeature.end())    //FaceUUID������
    //{
    //    nRet = FaceUUIDAleadyExist;
    //    printf("***Warning: CFeatureManage::AddFeature To KeyLib[%s] Failed, FaceUUID[%s] Aleady Exist.\n", m_pLibName, pFeatureData->pFaceUUID);
    //}
    //else
    {
        pKeyLibInfo->mapFeature.insert(make_pair(pFeatureData->pFaceUUID, pFeatureData));
        if (NULL == pKeyLibInfo->pHeadFeature) //��һ�β���
        {
            pKeyLibInfo->pHeadFeature = pFeatureData;
            pKeyLibInfo->pTailFeature = pFeatureData;
        }
        else
        {
            pKeyLibInfo->pTailFeature->pNext = pFeatureData;
            pFeatureData->pPrevious = pKeyLibInfo->pTailFeature;
            pKeyLibInfo->pTailFeature = pFeatureData;
        }

        pKeyLibInfo->nNum++;
    }
    if (!bInit)
    {
        printf("Add Feature To Lib[%s] Success, FaceUUID[%s].\n", m_pLibName, pFeatureData->pFaceUUID);
    }
        
    return nRet;
}


//ɾ������ֵ
int CFeatureManage::DelFeature(const char * pFaceUUID)
{
    int nRet = FaceUUIDNotExist;
    for (LISTKEYLIBINFO::iterator it = m_listKeyLibInfo.begin(); it != m_listKeyLibInfo.end(); it++)
    {
        MAPKEYLIBFEATUREDATA::iterator itFeature = (*it)->mapFeature.find(pFaceUUID);
        if ((*it)->mapFeature.end() == itFeature)
        {
            continue;
        }
        else
        {
            if (itFeature->second == (*it)->pHeadFeature)   //��ɾ��ͷָ��
            {
                (*it)->pHeadFeature = (*it)->pHeadFeature->pNext;   //ͷָ��ָ����һ�����
                if (NULL == (*it)->pHeadFeature)    //���ʱͷָ��Ϊ��, ��˵���޽��, βָ��Ҳ��Ϊ��
                {
                    (*it)->pTailFeature = NULL;
                }
                else//��ʱͷָ�벻Ϊ��
                {
                    (*it)->pHeadFeature->pPrevious = NULL;
                }
            }
            else   if (itFeature->second == (*it)->pTailFeature)   //��ɾ��βָ��
            {
                (*it)->pTailFeature = itFeature->second->pPrevious; //ָ��ָ��ǰһ�����
                (*it)->pTailFeature->pNext = NULL;
            }        
            else
            {
                itFeature->second->pPrevious->pNext = itFeature->second->pNext;
                itFeature->second->pNext->pPrevious = itFeature->second->pPrevious;
            }
            delete itFeature->second;
            (*it)->mapFeature.erase(itFeature);
            (*it)->nNum--;
            nRet = INVALIDERROR;
        }
    }
    
    return nRet;
}
int CFeatureManage::ClearKeyLib()
{
    while (m_listKeyLibInfo.size() > 0)
    {
        delete m_listKeyLibInfo.front();
        m_listKeyLibInfo.pop_front();
    }
    return 0;
}
unsigned int CFeatureManage::GetTotalNum()
{
    unsigned int nCount = 0;
    LISTKEYLIBINFO::iterator it = m_listKeyLibInfo.begin();
    for (; it != m_listKeyLibInfo.end(); it++)
    {
        nCount += (*it)->nNum;
    }

    return nCount;
}