#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "STSDKInterface.h"
#include "STAnalyseInterface.h"

#define THREADWAIT      10                  //�̵߳ȴ�ʱ��(ms)
#define LOADSTLIBMAX    5                   //��ST����ೢ�Դ���
#define FACEIMAGELEN    1024 * 1024 * 2     //faceͼƬ���Buf����
#define MAXFACEIMAGEINFO 150                //ԭʼ��Դ�������


//Redis Hash���ֶζ���
#define HASHPICTURE     "Picture"
#define HASHFACE        "Face"
#define HASHTIME        "Time"
#define HASHSCORE       "Score"
#define HASHFEATURE     "Feature"

#define SUBSCRIBEPATTERN    "Checkpoints."          //Redis����
#define LOCALFILESAVEPATH   "D:/StoreSTLibServer/"  //����������ͼƬ���ر���·��
#define SQLMAXLEN           1024 * 4                //SQL�����󳤶�

#define STTHREADNUM  8           //��ȡ����ֵ�����߳���
#define STTESTLIB   "BatchTemp"   //���������������������ʱ����

//���ݿ����
#define LAYOUTCHECKPOINTTABLE   "layoutcheckpoint"  //���ؿ��ڱ�
#define STORELIBTABLE           "storelib"          //�ص���
#define STOREFACEINFOTABLE      "storefaceinfo"     //�ص��ͼƬ��
#define LAYOUTRESULTTABLE       "layoutresult"      //���رȶԽ����
#define SERVERTYPETABLE         "servertype"        //�������ͱ�
#define SERVERINFOTABLE         "serverinfo"        //������Ϣ��

#define FEATUREMAXLEN   1024 * 4

enum ErrorCode      //�����붨��
{
    INVALIDERROR = 0,       //�޴���
    ServerNotInit = 1001,   //������δ��ʼ�����
    DBAleadyExist,          //�⼺����
    DBNotExist,             //�ⲻ����
    FaceUUIDAleadyExist,    //FaceUUID������
    FaceUUIDNotExist,       //FaceUUID������
    ParamIllegal,           //�����Ƿ�
    NewFailed,              //new�����ڴ�ʧ��
    JsonFormatError,        //json����ʽ����
    CommandNotFound,        //��֧�ֵ�����
    HttpMsgUpperLimit,      //http��Ϣ������������������
    PthreadMutexInitFailed, //�ٽ�����ʼ��ʧ��
    FeatureNumOverMax,      //������������ֵ��������
    JsonParamIllegal,       //Json����ֵ�Ƿ�
    MysqlQueryFailed,       //Mysql����ִ��ʧ��.
    VerifyFailed,           //�ȶ�ʧ��
    PushFeatureToAnalyseFailed, //����ֵ���͸���������ʧ��
    STSDKInitFailed,        //ST���ʼ��ʧ��
    STJsonFormatFailed,     //ST���ؽ��Json������ʧ��
    GetSTFeatureFailed,      //����ST�ص��ʧ��
    AddImageToSTFailed,     //����ͼƬ��ST��������ȡ����ֵ, ����errorʧ��
    STRepGetFeatureFailed,  //ST��������ȡ����ͼƬ����ֵʧ��
    STRepFileNameNotExist,  //���ST������, �����ļ���������
    GetLocalPathFailed,     //��ȡ���񱾵�·��ʧ��
    LoadLibFailed,          //���ؿ�ʧ��
    GetProcAddressFailed,   //��ȡ������ַʧ��
    FRSEngineInitFailed,    //��������ֵ��ȡ���ʼ��ʧ��
    ConvertBMPFailed,       //ͼƬתBMPʧ��
    GetLocalFeatureFailed   //���ػ�ȡͼƬ����ֵʧ��
};

typedef std::map<std::string, LPSTSERVERINFO> MAPSTSERVERINFO;
typedef std::map<std::string, LPSTANALYSESERVERINFO> MAPANALYSESERVERINFO;

#ifdef LAYOUTSERVER //���ط���
//���ؿ�����Ϣ
typedef std::map<std::string, unsigned int> MAPLAYOUTCHECKPOINT;
typedef struct _LayoutInfo
{
    int nLayoutLibID;       //���ؿ�ID
    char pBeginTime[32];    //��ʼʱ��
    char pEndTime[32];      //����ʱ��
    std::list<std::string> listFaceUUID;  //��Ҫ���ػ�ɾ�����ص�ͼƬFaceUUID
    MAPLAYOUTCHECKPOINT mapCheckpoint;    //���ؿ��ڱ������, һ����Ϣ����Я��������ڱ��
    _LayoutInfo()
    {
        nLayoutLibID = 0;
        ZeroMemory(pBeginTime, sizeof(pBeginTime));
        ZeroMemory(pEndTime, sizeof(pEndTime));
    }
    ~_LayoutInfo()
    {
        listFaceUUID.clear();
        mapCheckpoint.clear();
    }
}LAYOUTINFO, *LPLAYOUTINFO;
#endif

#ifdef BATCHSTORELIBSERVER     //�ص������������
typedef std::map<std::string, std::string> MAPZBASE64FACE;  //����ͼƬ����(*.jpg), ����ͼƬ��Ϣ(ZBase64�����ʽ)
typedef struct _StoreInfo
{
    int nStoreLibID;                        //�ص��ID
    std::list<std::string> listFaceUUID;    //��Ҫ����ɾ����ͼƬFaceUUID
    MAPZBASE64FACE mapZBase64FaceInfo;      //����ͼƬ��Ϣ(ZBase64�����ʽ)
    _StoreInfo()
    {
        nStoreLibID = 0;
    }
    ~_StoreInfo()
    {
        listFaceUUID.clear();
        mapZBase64FaceInfo.clear();
    }
}STOREINFO, *LPSTOREINFO;
#endif

typedef struct _HTTPRequest
{
    std::string sHttpHead;  //�ͻ�������Http Head
    std::string sHttpBody;  //�ͻ�������Http Body
    SOCKET ClientSocket;    //�ͻ���Socekt
    int nOperatorType;      //��������, 1: ��������ͼƬ, 2: ɾ������ͼƬ, 3: ���Ӳ��ؿ���, 4: ɾ�����ؿ���, 5: ɾ�����ؿ�
#ifdef BATCHSTORELIBSERVER
    LPSTOREINFO pStoreInfo;
#endif
#ifdef LAYOUTSERVER
    LPLAYOUTINFO pLayoutInfo;
#endif
    _HTTPRequest()
    {
        sHttpHead = "";
        sHttpBody = "";
        ClientSocket = 0;
#ifdef BATCHSTORELIBSERVER
        pStoreInfo = NULL;
#endif
#ifdef LAYOUTSERVER
        pLayoutInfo = NULL;
#endif
    }
    ~_HTTPRequest()
    {
#ifdef BATCHSTORELIBSERVER
        if (NULL != pStoreInfo)
        {
            delete pStoreInfo;
            pStoreInfo = NULL;
        }
#endif
#ifdef LAYOUTSERVER
        if (NULL != pLayoutInfo)
        {
            delete pLayoutInfo;
            pLayoutInfo = NULL;
        }
#endif
    }
}HTTPREQUEST, *LPHTTPREQUEST;
typedef std::list<LPHTTPREQUEST> LISTHTTPREQUEST;

#ifdef LAYOUTSERVER
//�����ϴ��������񲼿ؿ��ͼƬSTImageID��FaceUUID
typedef std::map<std::string, std::string> MAPLAYOUTFACE;
//���ط����Redis������Ϣ��
typedef std::map<std::string, std::string> MAPREDISSUBSCRIBE;

typedef struct _LayoutLibInfo
{
    unsigned int nLayoutLibID;              //���ؿ�ID
    char pBeginTime[20];                    //���ؿ�ʼʱ��
    char pEndTime[20];                      //���ؽ���ʱ��
    MAPLAYOUTCHECKPOINT mapLayoutCheckPoint;//���ؿ���
    MAPLAYOUTFACE mapLayoutFace;            //��������(FaceUUID, "")
    _LayoutLibInfo()
    {
        nLayoutLibID = 0;
        ZeroMemory(pBeginTime, sizeof(pBeginTime));
        ZeroMemory(pEndTime, sizeof(pEndTime));
        mapLayoutFace.clear();
        mapLayoutCheckPoint.clear();
    }
    ~_LayoutLibInfo()
    {
        mapLayoutFace.clear();
        mapLayoutCheckPoint.clear();
    }
}LAYOUTLIBINFO, *LPLAYOUTLIBINFO;
typedef std::map<unsigned int, LPLAYOUTLIBINFO> MAPLAYOUTLIBINFO;

//����ͼƬ�����Ϣ
typedef struct _FaceImageInfo
{
    char pFaceUUID[64];     //����ͼƬΨһUUID
    char pDeviceID[64];     //ץ����������
    char pImageTime[20];            //ץ��ʱ��
    char pSTImageFeature[FEATUREMAXLEN]; //����������ⷵ�ص���������
    int nScore;                     //ƥ������ͼƬ��ͷ���
    std::map<std::string, int> mapSearchImageID;    //����ƥ������, �������񷵻ص�ͼƬID�����map
    unsigned int nLayoutLibID;      //ST���ؿ�ID
    SOCKET ClientSocket;            //�ͻ���Socekt
    _FaceImageInfo()
    {
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pDeviceID, sizeof(pDeviceID));
        ZeroMemory(pImageTime, sizeof(pImageTime));
        ZeroMemory(pSTImageFeature, sizeof(pSTImageFeature));
        nScore = 0;
        nLayoutLibID = 0;
    }
    void Init()
    {
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pDeviceID, sizeof(pDeviceID));
        ZeroMemory(pImageTime, sizeof(pImageTime));
        ZeroMemory(pSTImageFeature, sizeof(pSTImageFeature));
        nScore = 0;
        mapSearchImageID.clear();
        nLayoutLibID = 0;
    }
    ~_FaceImageInfo()
    {
        mapSearchImageID.clear();
    }
}FACEIMAGEINFO, *LPFACEIMAGEINFO;
typedef std::list<LPFACEIMAGEINFO> LISTFACEIMAGEINFO;

//��Ϣ������ɺ�ص�����
typedef void (CALLBACK *pImageInfoCallback)(LPFACEIMAGEINFO pFaceImageInfo, void * pUser);
#endif

#ifdef BATCHSTORELIBSERVER
typedef std::map<std::string, std::string> MAPSTOREFACE;
typedef struct _StoreLibInfo
{
    unsigned int nStoreLibID;              //�ص��ID
    MAPSTOREFACE mapStoreFace;             //�����ص��������Ӧ��ϵ(FaceUUID, ImageID)
    _StoreLibInfo()
    {
        nStoreLibID = 0;
        mapStoreFace.clear();
    }
    ~_StoreLibInfo()
    {
        mapStoreFace.clear();
    }
}STORELIBINFO, *LPSTORELIBINFO;
typedef std::map<unsigned int, LPSTORELIBINFO> MAPSTORELIBINFO;

typedef struct _PushSTImageInfo
{
    char pImageName[64];    //ͼƬ��(*.jpg)
    char pFaceUUID[64];     //����ͼƬΨһUUID
    char * pImageBuf;       //ͼƬ��Ϣbuf
    int nImageBufMaxLen;    //ͼƬ��ϢBuf��󳤶�
    int nImageLen;          //ͼƬ��С
    char pSTImageID[64];    //��������ⷵ�ص�ͼƬID
    char pSTImageFeature[FEATUREMAXLEN]; //����������ⷵ�ص���������
    int nFaceQuality;       //�������񷵻ص���������, ���ʧ��ʱ���������
    bool bUsed;             //��ǰ�Ƿ񼺱�ʹ��
    char pFilePath[128];
    char pErrorMsg[64];
    _PushSTImageInfo()
    {
        ZeroMemory(pImageName, sizeof(pImageName));
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pSTImageID, sizeof(pSTImageID));
        ZeroMemory(pSTImageFeature, sizeof(pSTImageFeature));
        ZeroMemory(pFilePath, sizeof(pFilePath));
        ZeroMemory(pErrorMsg, sizeof(pErrorMsg));

        pImageBuf = new char[FACEIMAGELEN];
        nImageBufMaxLen = FACEIMAGELEN;
        nImageLen = 0;
        nFaceQuality = -1;
        bUsed = false;
    }
    void Init()
    {
        ZeroMemory(pImageName, sizeof(pImageName));
        ZeroMemory(pFaceUUID, sizeof(pFaceUUID));
        ZeroMemory(pSTImageID, sizeof(pSTImageID));
        ZeroMemory(pSTImageFeature, sizeof(pSTImageFeature));
        ZeroMemory(pFilePath, sizeof(pFilePath));
        ZeroMemory(pErrorMsg, sizeof(pErrorMsg));
        nImageLen = 0;
        nFaceQuality = -1;
        bUsed = false;
    }
    ~_PushSTImageInfo()
    {
        delete []pImageBuf;
    }
}PUSHSTIMAGEINFO, *LPPUSHSTIMAGEINFO;
typedef std::list<LPPUSHSTIMAGEINFO> LISTPUSHSTIMAGEINFO;

typedef struct _StoreFaceInfo
{
    unsigned int nStoreLibID;   //�ص��ID
    SOCKET ClientSocket;        //�ͻ���Socekt
    int nEvent;                 //�����������ص����, 0�ɹ�, < 0 ʧ��
    unsigned int nSuccessNum;   //�ɹ��������
    LISTPUSHSTIMAGEINFO listPushSTImageInfo;
    _StoreFaceInfo()
    {
        ClientSocket = INVALID_SOCKET;
        nSuccessNum = 0;
        nStoreLibID = 0;
        nEvent = 0;
    }
    void Init()
    {
        ClientSocket = INVALID_SOCKET;
        nSuccessNum = 0;
        nStoreLibID = 0;
        nEvent = 0;
        for(LISTPUSHSTIMAGEINFO::iterator it = listPushSTImageInfo.begin();
            it != listPushSTImageInfo.end(); it ++)
        {
            (*it)->Init();
        }
    }
    ~_StoreFaceInfo()
    {
        while(listPushSTImageInfo.size() > 0)
        {
            delete listPushSTImageInfo.front();
            listPushSTImageInfo.pop_front();
        }
    }
}STOREFACEINFO, *LPSTOREFACEINFO;
typedef std::list<LPSTOREFACEINFO> LISTSTOREFACEINFO;
typedef void (CALLBACK *pStoreImageCallback)(LPSTOREFACEINFO pStoreImageInfo, void * pUser);

#endif
