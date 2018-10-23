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
    string m_sCurrentPath;  //程序当前路径
    string m_sConfigFile;	//配置路径

    string m_sDBType;       //数据库类型
    string m_sDBIP;		    //数据库IP
    int m_nDBPort;          //数据库端口
    string m_sDBName;		//数据库名
    string m_sDBUid;		//用户名
    string m_sDBPwd;		//用户密码
};
