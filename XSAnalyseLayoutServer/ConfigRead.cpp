#include "stdafx.h"
#include "ConfigRead.h"


CConfigRead::CConfigRead(void)
{
    m_sDBIP = "";
    m_nDBPort = 0;
    m_sDBName = "";
    m_sDBUser = "";
    m_sDBPd = "";
    m_nLayoutSearchThreadCount = 0;
}
#ifdef __WINDOWS__
string CConfigRead::GetCurrentPath()
{
    DWORD nBufferLenth = MAX_PATH;
    char szBuffer[MAX_PATH] = { 0 };
    DWORD dwRet = GetModuleFileNameA(NULL, szBuffer, nBufferLenth);
    char *sPath = strrchr(szBuffer, '\\');
    memset(sPath, 0, strlen(sPath));
    m_sCurrentPath = szBuffer;
    return m_sCurrentPath;
}
#endif
CConfigRead::~CConfigRead(void)
{
}
bool CConfigRead::ReadConfig()
{
#ifdef __WINDOWS__
    GetCurrentPath();
    m_sConfigFile = m_sCurrentPath + "/Config/config.txt";
#ifdef _DEBUG
    m_sConfigFile = "./Config/config.txt";
#endif
#else
    m_sConfigFile = "config.txt";
#endif
    fstream cfgFile;  
    cfgFile.open(m_sConfigFile.c_str()); //���ļ�      
    if(!cfgFile.is_open())  
    {  
        printf("can not open cfg file!\n",  m_sConfigFile.c_str()); 
        return false;  
    }  

    char tmp[1000];  
    while(!cfgFile.eof())//ѭ����ȡÿһ��  
    {  
        cfgFile.getline(tmp, 1000);//ÿ�ж�ȡǰ1000���ַ���1000��Ӧ���㹻��  
        string sLine(tmp);  
        size_t pos = sLine.find('=');//�ҵ�ÿ�еġ�=����λ�ã�֮ǰ��key֮����value  
        if(pos == string::npos) 
        {
            printf("****Error: Config File Format Wrong: [%s]!\n", sLine.c_str());
            return false;  
        }
        string tmpKey = sLine.substr(0, pos); //ȡ=��֮ǰ  
        if(sLine[sLine.size() - 1] == 13)
        {
            sLine.erase(sLine.size() -1, 1);
        }

        if ("LayoutServerID" == tmpKey)
        {
            m_sServerID.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        }
        else if ("ThreadMaxCount" == tmpKey)
        {
            string sCount = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nMaxCount = atoi(sCount.c_str());
        }
        else if("DBIP" == tmpKey)  
        {  
            m_sDBIP.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        }  
        else if("DBPort" == tmpKey)  
        {  
            string sDBPort = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nDBPort = atoi(sDBPort.c_str());
        } 
        else if("DBLayoutName" == tmpKey)  
        {  
            m_sDBName.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        }  
        else if("DBUser" == tmpKey)  
        {  
            m_sDBUser.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        } 
        else if("DBPd" == tmpKey)  
        {  
            m_sDBPd.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        } 
        else if ("PubServerIP" == tmpKey)
        {
            m_sPubServerIP.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        }
        else if ("PubServerPort" == tmpKey)
        {
            string sPort = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nPubServerPort = atoi(sPort.c_str());
        }
        else if ("SubServerPort" == tmpKey)
        {
            string sPort = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nSubServerPort = atoi(sPort.c_str());
        }
        else if ("LayoutPushIP" == tmpKey)
        {
            m_sLayoutPushIP.assign(sLine, pos + 1, sLine.size() - 1 - pos);
        }
        else if ("LayoutPushPort" == tmpKey)
        {
            string sPort = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nLayoutPushPort = atoi(sPort.c_str());
        }
        else if ("LayoutPullPort" == tmpKey)
        {
            string sPort = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nLayoutPullPort = atoi(sPort.c_str());
        }
        else if ("LayoutSearchThreadCount" == tmpKey)
        {
            string sCount = sLine.substr(pos + 1);//ȡ=��֮��  
            m_nLayoutSearchThreadCount = atoi(sCount.c_str());
            break;
        }
    }  
    printf("DBInfo: %s:%d:%s\n", m_sDBIP.c_str(), m_nDBPort, m_sDBName.c_str());
    if("" == m_sDBIP || 0 == m_nDBPort || "" == m_sDBName || "" == m_sDBUser || "" == m_sDBPd)
    {
        printf("****Error: Get DB Info Failed!\n");
        return false;
    }

    return true;
}
