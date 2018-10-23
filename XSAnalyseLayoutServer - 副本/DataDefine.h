#ifndef datadefine_h

#include <map>
#include <string>
#include <list>
using namespace std;

//���ݿ����
#define LIBINFOTABLE        "layoutlibinfo"

#define COMMANDADDLIB "addlib"    //���ӹ������ڻ��ص��
#define COMMANDDELLIB "dellib"    //ɾ���������ڻ��ص��
#define COMMANDADD      "add"       //��������ֵ����
#define COMMANDDEL      "del"       //ɾ������ֵ����
#define COMMANDCLEAR    "clear"     //�������ֵ������
#define COMMANDSEARCH   "search"    //��������ֵ����

//Redis Hash���ֶζ���
#define REDISDEVICEID   "libid"     //����ID���ص��ID
#define REDISFACEUUID   "faceuuid"  //faceuuid
#define REDISSCORE      "score"     //����
#define REDISTIME       "time"      //ʱ��
#define REDISDRIVE      "drive"     //�̷�
#define REDISIMAGEIP    "imageip"   //ͼƬ���������IP
#define REDISHITCOUNT   "hitcount"  //ͼƬ���д���
#define REDISFACERECT   "facerect"  //��������
#define HASHOVERTIME    1800        //���ݱ���ʱ��

#define SUBALLCHECKPOINT    "allcheckpoints"    //�������п���
#define SUBALLLIB           "allfocuslib"       //���������ص��

#define DEVICESTATUS    "libstatus"  //����״̬����
#define LAYOUTLIBINFO    "layoutlibinfo"  //���ؿ���Ϣ����

//Json���ֶζ���
#define JSONDLIBID "libid"              //���ڻ��ص��ID
//����
#define JSONDEVICEID    "checkpoint"    //����ID
#define JSONFACDUUID    "faceuuid"      //����ֵFaceUUID
#define JSONFACERECT    "facerect"      //����ͼƬ����
#define JSONFEATURE     "feature"       //����ֵ
#define JSONTIME        "imagetime"     //ץ��ʱ��
#define JSONDRIVE       "imagedisk"     //ͼƬ�����ڲɼ������ϵ������̷�
#define JSONSERVERIP    "imageip"       //�ɼ�����IP

#define JSONTASKID      "taskid"        //��������ID
#define JSONFACE        "faces"         //��������ֵ����
#define JSONFACENO      "faceno"
#define JSONFACEFEATURE "feature"
#define JSONBEGINTIME   "startime"      //������ʼʱ��
#define JSONENDTIME     "endtime"       //��������ʱ��
#define JSONSCORE       "score"         //������ֵ
#define JSONADDRESS     "address"       //���ؿ�ָ���ַ
#define JSONLAYOUTFACDUUID    "layoutfaceuuid"      //ƥ���еĲ�������ֵ

#define JSONSEARCHCOUNT "count"         //���������
#define JSONSEARCHHIGHEST "highest"     //������߷���
#define JSONSEARCHLATESTTIME "lasttime" //��������ͼƬʱ��

#define SQLMAXLEN       1024 * 4    //SQL�����󳤶�
#define MAXIPLEN        20          //IP, FaceRect��󳤶�
#define MAXLEN          36          //FaceUUID, LibName, DeviceID��󳤶�
#define FEATURELEN      1024 * 4    //Feature��󳤶�
#define FEATUREMIXLEN   500         //Feature��̳���, С�ڴ˳��ȶ�Ϊ���Ϸ�
#define RESOURCEMAXNUM  150         //���HTTP��Ϣ������, �������޷�������д���, ֱ�ӷ���ʧ��
#define THREADNUM       8           //�߳���
#define THREADWAITTIME  5           //�̵߳ȴ�ʱ��(ms)
#define FEATURESEARCHWAIT   2000    //���߳������ȴ���ʱʱ��

enum ErrorCode
{
    INVALIDERROR = 0,       //�޴���
    ServerNotInit = 12001,  //������δ��ʼ�����
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
    ParamLenOverMax,        //��������̫��
    LibAleadyExist,         //�⼺����
    LibNotExist,            //�ⲻ����
    CheckpointInitFailed,   //�������ʼ��ʧ��
    VerifyFailed,           //����ֵ�ȶ�ʧ��, ʧ��һ�κ󼴲��ٱȶ�
    HttpSocketBindFailed,   //http�˿ڰ�ʧ��
    CreateTableFailed,      //�����ݿⴴ����ʧ��
    SearchTimeWrong,        //�����ʱ���(��ʼʱ����ڽ���ʱ��)
    SearchNoDataOnTime,      //ָ��ʱ�����û������
    AddFeatureToCheckpointFailed,    //��������ֵ������DB����
    SocketInitFailed,        //�����ʼ��ʧ��
    InsertRedisFailed       //���ݲ���Redis����
};

//��������
enum TASKTYPE
{
    INVALIDTASK,
    ADDLIB,             //���Ӳ��ؿ�
    DELLIB,             //ɾ�����ؿ�
    LIBADDFEATURE,      //��������ֵ
    LIBDELFEATURE,      //ɾ������ֵ
    LIBCLEARFEATURE,    //�������ֵ
    LIBSEARCH,          //��������ֵ
};

typedef struct _DeviceInfo
{
    int nServerType;    //1: ���ڷ�������, 2: ������������, 3: ���ط�������
    string sLibID;
    int nMaxCount;
    string sDBIP;
    int nDBPort;
    string sDBName;
    string sDBUser;
    string sDBPassword;
    string sPubServerIP;
    int nPubServerPort;
    int nSubServerPort;
    string sRedisIP;
    int nRedisPort;
}DEVICEINFO, *LPDEVICEINFO;

typedef struct _SubMessage
{
    char pHead[MAXLEN];         //������Ϣͷ
    char pOperationType[MAXLEN];//������Ϣ��������
    char pSource[MAXLEN];       //������ϢԴ
    char pSubJsonValue[FEATURELEN * 10]; //������ϢJson��
    string sPubJsonValue;       //������ϢJson��

    int nTaskType;              //��������

    char pFaceUUID[MAXLEN];     //����ֵFaceUUID
    char pFeature[FEATURELEN];  //����ֵ
    char pDeviceID[MAXLEN];     //����ID
    int nImageTime;             //ץ��ʱ��
    char pDisk[2];              //����ֵ�������
    char pImageIP[MAXIPLEN];    //����ֵ���������IP
    char pFaceRect[MAXIPLEN];   //ͼƬ��������
    int nScore;                 //������ֵ
    int nAddress;               //���ؿ�ָ���ַ
}SUBMESSAGE, *LPSUBMESSAGE;
typedef std::list<LPSUBMESSAGE> LISTSUBMESSAGE;
typedef void(*LPSubMessageCallback)(LPSUBMESSAGE pSubMessage, void * pUser);


//�ص������ֵ���
typedef struct _KeyLibFeatureData
{
    char * pFaceUUID;           //����ֵFaceUUID
    char * pFeature;            //����ֵ
    int nFeatureLen;			//���������ֵ����
    char pDisk;
    char * pImageIP;
    _KeyLibFeatureData * pNext;       //��һ�����
    _KeyLibFeatureData * pPrevious;   //ǰһ�����
    _KeyLibFeatureData()
    {
        pFaceUUID = NULL;
        pFeature = NULL;
        pNext = NULL;
        pPrevious = NULL;
    }
    ~_KeyLibFeatureData()
    {
        if (NULL != pFaceUUID)
        {
            delete[]pFaceUUID;
            pFaceUUID = NULL;
        }
        if (NULL != pFeature)
        {
            delete[]pFeature;
            pFeature = NULL;
        }
        if (NULL != pImageIP)
        {
            delete[]pImageIP;
            pImageIP = NULL;
        }
        pNext = NULL;
        pPrevious = NULL;
    }
}KEYLIBFEATUREDATA, *LPKEYLIBFEATUREDATA;
typedef map<string, LPKEYLIBFEATUREDATA> MAPKEYLIBFEATUREDATA;

//�ص����
typedef struct _KeyLibInfo
{
    char pLibID[MAXLEN];        //�ص����
    unsigned int nNum;          //����ֵ�������
    LPKEYLIBFEATUREDATA pHeadFeature; //����ֵ����ͷ
    LPKEYLIBFEATUREDATA pTailFeature; //����ֵ����β
    MAPKEYLIBFEATUREDATA mapFeature;  //����FaceUUID��Ӧ����ֵ���map
    _KeyLibInfo()
    {
        memset(pLibID, 0, sizeof(pLibID));
        nNum = 0;
        pHeadFeature = NULL;
        pTailFeature = NULL;
        mapFeature.clear();
    }
    ~_KeyLibInfo()
    {
        nNum = 0;
        while (NULL != pHeadFeature)
        {
            LPKEYLIBFEATUREDATA pFeatureData = pHeadFeature->pNext;
            delete pHeadFeature;
            pHeadFeature = pFeatureData;
        }
        mapFeature.clear();
    }
}KEYLIBINFO, *LPKEYLIBINFO;
typedef list<LPKEYLIBINFO> LISTKEYLIBINFO;
typedef map<int, LPKEYLIBINFO> MAPKEYLIBINFO;


//�����ص����������ֵ��Ƭ�����Ϣ
typedef struct _SingleFeatureNode
{
    MAPKEYLIBINFO mapSingleFeatureNode;
}SINGLEFEATURENODE, *LPSINGLEFEATURENODE;
typedef map<string, LPSINGLEFEATURENODE> MAPSINGLEFEATURENODE;

//�����ص����������ֵ��Ƭ�����Ϣ
typedef struct _AllFeatureNodeInfo
{
    MAPSINGLEFEATURENODE mapAllFeatureNodeInfo;
}ALLFEATURENODEINFO, *LPALLFEATURENODEINFO;

#define datadefine_h
#endif


