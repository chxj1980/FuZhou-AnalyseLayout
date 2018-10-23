#ifndef configread_h

#include <iostream>  
#include <string>  
#include <fstream> 
#include <stdio.h>
#include <stdlib.h>
using namespace std;

class CConfigRead
{
public:
    CConfigRead(void);
    ~CConfigRead(void);
public:
#ifdef __WINDOWS__
    string GetCurrentPath();
#endif
    bool ReadConfig();
public:
    string m_sConfigFile;
    string m_sCurrentPath;

    string m_sServerID;
    int m_nMaxCount;    //ÿ50w���ݶ��ⴴ��һ�������߳�
    string m_sDBIP;
    int m_nDBPort;
    string m_sDBName;
    string m_sDBUser;
    string m_sDBPd;

    string m_sPubServerIP;
    int m_nPubServerPort;
    int m_nSubServerPort;

    string m_sLayoutPushIP; //���ط���IP
    int m_nLayoutPushPort;  //���ط�����������ֵ��Ϣ�˿�
    int m_nLayoutPullPort;  //���ط�����ջ�Ӧ��Ϣ�˿�
};

#define configread_h
#endif