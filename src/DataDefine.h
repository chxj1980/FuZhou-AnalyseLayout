#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "STSDKInterface.h"
#include "STAnalyseInterface.h"

#define THREADWAIT      10                  //线程等待时间(ms)
#define LOADSTLIBMAX    5                   //入ST库最多尝试次数
#define FACEIMAGELEN    1024 * 1024 * 2     //face图片最大Buf长度
#define MAXFACEIMAGEINFO 150                //原始资源池最大数


//Redis Hash表字段定义
#define HASHPICTURE     "Picture"
#define HASHFACE        "Face"
#define HASHTIME        "Time"
#define HASHSCORE       "Score"
#define HASHFEATURE     "Feature"

#define SUBSCRIBEPATTERN    "Checkpoints."          //Redis订阅
#define LOCALFILESAVEPATH   "D:/StoreSTLibServer/"  //批量入库服务图片本地保存路径
#define SQLMAXLEN           1024 * 4                //SQL语句最大长度

#define STTHREADNUM  8           //获取特征值处理线程数
#define STTESTLIB   "BatchTemp"   //批量入库服务入分析服务临时库名

//数据库表名
#define LAYOUTCHECKPOINTTABLE   "layoutcheckpoint"  //布控卡口表
#define STORELIBTABLE           "storelib"          //重点库表
#define STOREFACEINFOTABLE      "storefaceinfo"     //重点库图片表
#define LAYOUTRESULTTABLE       "layoutresult"      //布控比对结果表
#define SERVERTYPETABLE         "servertype"        //服务类型表
#define SERVERINFOTABLE         "serverinfo"        //服务信息表

#define FEATUREMAXLEN   1024 * 4

enum ErrorCode      //错误码定义
{
    INVALIDERROR = 0,       //无错误
    ServerNotInit = 1001,   //服务尚未初始化完成
    DBAleadyExist,          //库己存在
    DBNotExist,             //库不存在
    FaceUUIDAleadyExist,    //FaceUUID己存在
    FaceUUIDNotExist,       //FaceUUID不存在
    ParamIllegal,           //参数非法
    NewFailed,              //new申请内存失败
    JsonFormatError,        //json串格式错误
    CommandNotFound,        //不支持的命令
    HttpMsgUpperLimit,      //http消息待处理数量己达上限
    PthreadMutexInitFailed, //临界区初始化失败
    FeatureNumOverMax,      //批量增加特征值数量超标
    JsonParamIllegal,       //Json串有值非法
    MysqlQueryFailed,       //Mysql操作执行失败.
    VerifyFailed,           //比对失败
    PushFeatureToAnalyseFailed, //特征值推送给分析服务失败
    STSDKInitFailed,        //ST库初始化失败
    STJsonFormatFailed,     //ST返回结果Json串解析失败
    GetSTFeatureFailed,      //创建ST重点库失败
    AddImageToSTFailed,     //推送图片给ST服务器获取特征值, 返回error失败
    STRepGetFeatureFailed,  //ST服务器获取单张图片特征值失败
    STRepFileNameNotExist,  //入库ST服务器, 返回文件名不存在
    GetLocalPathFailed,     //获取服务本地路径失败
    LoadLibFailed,          //加载库失败
    GetProcAddressFailed,   //获取函数地址失败
    FRSEngineInitFailed,    //本地特征值获取库初始化失败
    ConvertBMPFailed,       //图片转BMP失败
    GetLocalFeatureFailed   //本地获取图片特征值失败
};

typedef std::map<std::string, LPSTSERVERINFO> MAPSTSERVERINFO;
typedef std::map<std::string, LPSTANALYSESERVERINFO> MAPANALYSESERVERINFO;

#ifdef LAYOUTSERVER //布控服务
//布控卡口信息
typedef std::map<std::string, unsigned int> MAPLAYOUTCHECKPOINT;
typedef struct _LayoutInfo
{
    int nLayoutLibID;       //布控库ID
    char pBeginTime[32];    //开始时间
    char pEndTime[32];      //结束时间
    std::list<std::string> listFaceUUID;  //需要布控或删除布控的图片FaceUUID
    MAPLAYOUTCHECKPOINT mapCheckpoint;    //布控卡口编号链表, 一次消息可能携带多个卡口编号
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

#ifdef BATCHSTORELIBSERVER     //重点库批量入库服务
typedef std::map<std::string, std::string> MAPZBASE64FACE;  //人脸图片名称(*.jpg), 人脸图片信息(ZBase64编码格式)
typedef struct _StoreInfo
{
    int nStoreLibID;                        //重点库ID
    std::list<std::string> listFaceUUID;    //需要入库或删除的图片FaceUUID
    MAPZBASE64FACE mapZBase64FaceInfo;      //人脸图片信息(ZBase64编码格式)
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
    std::string sHttpHead;  //客户端请求Http Head
    std::string sHttpBody;  //客户端请求Http Body
    SOCKET ClientSocket;    //客户端Socekt
    int nOperatorType;      //请求类型, 1: 增加人脸图片, 2: 删除人脸图片, 3: 增加布控卡口, 4: 删除布控卡口, 5: 删除布控库
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
//布控上传分析服务布控库的图片STImageID与FaceUUID
typedef std::map<std::string, std::string> MAPLAYOUTFACE;
//布控服务从Redis订阅消息池
typedef std::map<std::string, std::string> MAPREDISSUBSCRIBE;

typedef struct _LayoutLibInfo
{
    unsigned int nLayoutLibID;              //布控库ID
    char pBeginTime[20];                    //布控开始时间
    char pEndTime[20];                      //布控结束时间
    MAPLAYOUTCHECKPOINT mapLayoutCheckPoint;//布控卡口
    MAPLAYOUTFACE mapLayoutFace;            //布控人脸(FaceUUID, "")
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

//人脸图片相关信息
typedef struct _FaceImageInfo
{
    char pFaceUUID[64];     //人脸图片唯一UUID
    char pDeviceID[64];     //抓拍摄像机编号
    char pImageTime[20];            //抓拍时间
    char pSTImageFeature[FEATUREMAXLEN]; //分析服务入库返回的人脸特征
    int nScore;                     //匹配人脸图片最低分数
    std::map<std::string, int> mapSearchImageID;    //搜索匹配人脸, 分析服务返回的图片ID与分数map
    unsigned int nLayoutLibID;      //ST布控库ID
    SOCKET ClientSocket;            //客户端Socekt
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

//消息处理完成后回调函数
typedef void (CALLBACK *pImageInfoCallback)(LPFACEIMAGEINFO pFaceImageInfo, void * pUser);
#endif

#ifdef BATCHSTORELIBSERVER
typedef std::map<std::string, std::string> MAPSTOREFACE;
typedef struct _StoreLibInfo
{
    unsigned int nStoreLibID;              //重点库ID
    MAPSTOREFACE mapStoreFace;             //己入重点库人脸对应关系(FaceUUID, ImageID)
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
    char pImageName[64];    //图片名(*.jpg)
    char pFaceUUID[64];     //人脸图片唯一UUID
    char * pImageBuf;       //图片信息buf
    int nImageBufMaxLen;    //图片信息Buf最大长度
    int nImageLen;          //图片大小
    char pSTImageID[64];    //分析服务库返回的图片ID
    char pSTImageFeature[FEATUREMAXLEN]; //分析服务入库返回的人脸特征
    int nFaceQuality;       //分析服务返回的人脸分数, 入库失败时保存错误码
    bool bUsed;             //当前是否己被使用
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
    unsigned int nStoreLibID;   //重点库ID
    SOCKET ClientSocket;        //客户端Socekt
    int nEvent;                 //分析服务入重点库结果, 0成功, < 0 失败
    unsigned int nSuccessNum;   //成功入库数量
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
