// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#ifndef stdafx_h



#ifdef __WINDOWS__

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#pragma warning(disable:4996)
#pragma warning(disable:4099)
#pragma warning(disable:4244)
#pragma warning(disable:4003)
#pragma warning(disable:4267)
#pragma warning(disable:4018)
#pragma warning(disable:4267)

#include "AnalyseServer.h"
#include "FeatureManage.h"
#include "DataDefine.h"
#include "ConfigRead.h"
#include "ZBase64.h"
#include "LogRecorder.h"

#pragma comment( lib, "libzmq-v100-mt-gd-4_0_4.lib" )

#endif




#define stdafx_h
#endif

