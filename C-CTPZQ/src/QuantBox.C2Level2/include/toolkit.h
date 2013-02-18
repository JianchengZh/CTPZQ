#ifndef _TOOLKIT_H_
#define _TOOLKIT_H_

#include "Level2\level2_ns.h"
#include "Level2\ThostFtdcLevel2UserApiStruct.h"

#include <vector>
#include <set>
#include <string>

using namespace std;

_USING_LEVEL2_NS_

//����·�������ɶ༶Ŀ¼
void makedirs(const char* dir);

//���ַ�����ָ���ַ��ָ�
void GetSetFromString(const char* szString,const char* seps,vector<string>& vct,set<string>& st);

//����OnFrontDisconnected(int nReason)��ֵ���ϴ�����Ϣ
void GetOnFrontDisconnectedMsg(CThostFtdcRspInfoField* pRspInfo);

#endif