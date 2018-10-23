#pragma once
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/typeof/typeof.hpp>

using namespace boost::property_tree;
using namespace std;


class CConfigRead
{
public:
	CConfigRead(void);
public:
    ~CConfigRead(void);
public:
    string GetCurrentPath();
    bool ReadConfig();
public:
    string m_sCurrentPath;  //����ǰ·��
    string m_sConfigFile;	//����·��

    string m_sDBType;       //���ݿ�����
    string m_sDBIP;		    //���ݿ�IP
    int m_nDBPort;          //���ݿ�˿�
    string m_sDBName;		//���ݿ���
    string m_sDBUid;		//�û���
    string m_sDBPwd;		//�û�����
};
