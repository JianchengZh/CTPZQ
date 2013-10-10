#include "stdafx.h"
#include "Level2UserApi.h"
#include "Level2MsgQueue.h"
#include "include\toolkit.h"
#include "include\Lock.h"

#include <iostream>
using namespace std;

CLevel2UserApi::CLevel2UserApi(void)
{
	m_msgQueue = NULL;
	m_status = E_uninit;
	m_nRequestID = 0;

	InitializeCriticalSection(&m_csMapSecurityIDs);
	InitializeCriticalSection(&m_csMapIndexIDs);
}

CLevel2UserApi::~CLevel2UserApi(void)
{
	Disconnect();

	DeleteCriticalSection(&m_csMapSecurityIDs);
	DeleteCriticalSection(&m_csMapIndexIDs);
}

void CLevel2UserApi::RegisterMsgQueue(CLevel2MsgQueue* pMsgQueue)
{
	m_msgQueue = pMsgQueue;
}

bool CLevel2UserApi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)   
{
	bool bRet = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if(bRet)
	{
		if(m_msgQueue)
			m_msgQueue->Input_OnRspError(this,pRspInfo,nRequestID,bIsLast);
	}
	return bRet;
}

bool CLevel2UserApi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)   
{
	bool bRet = ((pRspInfo) && (pRspInfo->ErrorID != 0));

	return bRet;
}

void CLevel2UserApi::Connect(const string& szPath,
		const string& szAddresses,
		const string& szBrokerId,
		const string& szInvestorId,
		const string& szPassword)
{
	m_szBrokerId = szBrokerId;
	m_szInvestorId = szInvestorId;
	m_szPassword = szPassword;
	
	char *pszPath = new char[szPath.length()+20];
	sprintf(pszPath,"%s\\L2\\",szPath.c_str());
	makedirs(pszPath);

	m_pApi = CThostFtdcLevel2UserApi::CreateFtdcLevel2UserApi(pszPath);
	delete[] pszPath;

	m_status = E_inited;
	if(m_msgQueue)
		m_msgQueue->Input_OnConnect(this,NULL,m_status);
	
	if (m_pApi)
	{
		m_pApi->RegisterSpi(this);
		
		//��ӵ�ַ
		size_t len = szAddresses.length()+1;
		char* buf = new char[len];
		strncpy(buf,szAddresses.c_str(),len);

		char* token = strtok(buf, _QUANTBOXC2LEVEL2_SEPS_);
		while(token)
		{
			if (strlen(token)>0)
			{
				m_pApi->RegisterFront(token);
			}
			token = strtok( NULL, _QUANTBOXC2LEVEL2_SEPS_);
		}
		delete[] buf;
		
		//��ʼ������
		m_pApi->Init();
		m_status = E_connecting;
		if(m_msgQueue)
			m_msgQueue->Input_OnConnect(this,NULL,m_status);
	}
}

void CLevel2UserApi::ReqUserLogin()
{
	if (NULL == m_pApi)
		return;

	CThostFtdcReqUserLoginField request = {0};
	
	strncpy(request.BrokerID, m_szBrokerId.c_str(),sizeof(TThostFtdcBrokerIDType));
	strncpy(request.UserID, m_szInvestorId.c_str(),sizeof(TThostFtdcUserIDType));
	strncpy(request.Password, m_szPassword.c_str(),sizeof(TThostFtdcPasswordType));
	
	//ֻ����һ���õ���m_nRequestID��û�б�Ҫÿ������m_nRequestID����0��ʼ
	m_pApi->ReqUserLogin(&request,++m_nRequestID);
	m_status = E_logining;
	if(m_msgQueue)
		m_msgQueue->Input_OnConnect(this,NULL,m_status);
}

void CLevel2UserApi::Disconnect()
{
	m_status = E_unconnected;
	if(m_pApi)
	{
		m_pApi->RegisterSpi(NULL);
		m_pApi->Release();
		m_pApi = NULL;

		if(m_msgQueue)
			m_msgQueue->Input_OnDisconnect(this,NULL,m_status);
	}
}

void CLevel2UserApi::OnFrontConnected()
{
	m_status =  E_connected;
	if(m_msgQueue)
		m_msgQueue->Input_OnConnect(this,NULL,m_status);

	//���ӳɹ����Զ������¼
	ReqUserLogin();
}

void CLevel2UserApi::OnFrontDisconnected(int nReason)
{
	m_status = E_unconnected;
	CThostFtdcRspInfoField RspInfo;
	//����ʧ�ܷ��ص���Ϣ��ƴ�Ӷ��ɣ���Ҫ��Ϊ��ͳһ���
	RspInfo.ErrorID = nReason;
	GetOnFrontDisconnectedMsg(&RspInfo);
	
	if(m_msgQueue)
		m_msgQueue->Input_OnDisconnect(this,&RspInfo,m_status);
}

void CLevel2UserApi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo)
		&&pRspUserLogin)
	{
		m_status = E_logined;
		if(m_msgQueue)
			m_msgQueue->Input_OnConnect(this,pRspUserLogin,m_status);
		
		//�п��ܶ����ˣ������Ƕ������������¶���
		map<string,set<string> > mapOld = m_mapSecurityIDs;//�����ϴζ��ĵĺ�Լ
		
		for(map<string,set<string> >::iterator i=mapOld.begin();i!=mapOld.end();++i)
		{
			string strkey = i->first;
			set<string> setValue = i->second;

			SubscribeLevel2MarketData(setValue,strkey);//����
		}

		mapOld = m_mapIndexIDs;//�����ϴζ��ĵĺ�Լ
		
		for(map<string,set<string> >::iterator i=mapOld.begin();i!=mapOld.end();++i)
		{
			string strkey = i->first;
			set<string> setValue = i->second;

			SubscribeNGTSIndex(setValue,strkey);//����
		}
	}
	else
	{
		m_status = E_authed;
		if(m_msgQueue)
			m_msgQueue->Input_OnDisconnect(this,pRspInfo,E_logining);
	}
}

void CLevel2UserApi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(m_msgQueue)
		m_msgQueue->Input_OnRspError(this,pRspInfo,nRequestID,bIsLast);
}

void CLevel2UserApi::SubscribeLevel2MarketData(const string& szInstrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	vector<char*> vct;

	size_t len = szInstrumentIDs.length()+1;
	char* buf = new char[len];
	strncpy(buf,szInstrumentIDs.c_str(),len);

	len = szExchageID.length()+1;
	char* pExchageID = new char[len];
	strncpy(pExchageID,szExchageID.c_str(),len);

	CLock cl(&m_csMapSecurityIDs);

	set<string> _setInstrumentIDs;
	map<string,set<string> >::iterator it = m_mapSecurityIDs.find(szExchageID);
	if (it!=m_mapSecurityIDs.end())
	{
		_setInstrumentIDs = it->second;
	}

	char* token = strtok(buf, _QUANTBOXC2LEVEL2_SEPS_);
	while(token)
	{
		size_t l = strlen(token);
		if (l>0)
		{
			//��Լ�Ѿ����ڣ������ٶ��ģ�����ζ���Ҳû��ϵ
			_setInstrumentIDs.insert(token);//��¼�˺�Լ���ж���
			vct.push_back(token);
		}
		token = strtok( NULL, _QUANTBOXC2LEVEL2_SEPS_);
	}

	m_mapSecurityIDs[szExchageID] = _setInstrumentIDs;
	
	if(vct.size()>0)
	{
		//ת���ַ�������
		CThostFtdcSpecificSecurityField* pArray = new CThostFtdcSpecificSecurityField[vct.size()];
		for (size_t j = 0; j<vct.size(); ++j)
		{
			strncpy(pArray[j].SecurityID,vct[j],sizeof(TThostFtdcSecurityIDType));
			strncpy(pArray[j].ExchangeID,pExchageID,sizeof(TThostFtdcSecurityIDType));
		}

		//����
		m_pApi->SubscribeLevel2MarketData(pArray,(int)vct.size());

		delete[] pArray;
	}

	//�ͷ��ڴ�
	delete[] buf;
	delete[] pExchageID;
}

void CLevel2UserApi::UnSubscribeLevel2MarketData(const string& szInstrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	vector<char*> vct;

	size_t len = szInstrumentIDs.length()+1;
	char* buf = new char[len];
	strncpy(buf,szInstrumentIDs.c_str(),len);

	len = szExchageID.length()+1;
	char* pExchageID = new char[len];
	strncpy(pExchageID,szExchageID.c_str(),len);

	CLock cl(&m_csMapSecurityIDs);

	set<string> _setInstrumentIDs;
	map<string,set<string> >::iterator it = m_mapSecurityIDs.find(szExchageID);
	if (it!=m_mapSecurityIDs.end())
	{
		_setInstrumentIDs = it->second;
	}

	char* token = strtok(buf, _QUANTBOXC2LEVEL2_SEPS_);
	while(token)
	{
		size_t l = strlen(token);
		if (l>0)
		{
			//��Լ�Ѿ������ڣ�������ȡ�����ģ������ȡ��Ҳûʲô��ϵ
			_setInstrumentIDs.erase(token);//��¼�˺�Լ���ж���
			vct.push_back(token);
		}
		token = strtok( NULL, _QUANTBOXC2LEVEL2_SEPS_);
	}

	m_mapSecurityIDs[szExchageID] = _setInstrumentIDs;
	
	if(vct.size()>0)
	{
		//ת���ַ�������
		CThostFtdcSpecificSecurityField* pArray = new CThostFtdcSpecificSecurityField[vct.size()];
		for (size_t j = 0; j<vct.size(); ++j)
		{
			strncpy(pArray[j].SecurityID,vct[j],sizeof(TThostFtdcSecurityIDType));
			strncpy(pArray[j].ExchangeID,pExchageID,sizeof(TThostFtdcSecurityIDType));
		}

		//����
		m_pApi->UnSubscribeLevel2MarketData(pArray,(int)vct.size());

		delete[] pArray;
	}

	//�ͷ��ڴ�
	delete[] buf;
	delete[] pExchageID;
}

void CLevel2UserApi::SubscribeLevel2MarketData(const set<string>& instrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	string szInstrumentIDs;
	for(set<string>::iterator i=instrumentIDs.begin();i!=instrumentIDs.end();++i)
	{
		szInstrumentIDs.append(*i);
		szInstrumentIDs.append(";");
	}

	if (szInstrumentIDs.length()>1)
	{
		SubscribeLevel2MarketData(szInstrumentIDs,szExchageID);
	}
}

void CLevel2UserApi::OnRspSubLevel2MarketData(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//��ģ��ƽ̨��������������ᴥ��������Ҫ�Լ�ά��һ���Ѿ����ĵĺ�Լ�б�
	if(!IsErrorRspInfo(pRspInfo,nRequestID,bIsLast)
		&&pSpecificSecurity)
	{
		CLock cl(&m_csMapSecurityIDs);

		set<string> _setInstrumentIDs;
		map<string,set<string> >::iterator it = m_mapSecurityIDs.find(pSpecificSecurity->ExchangeID);
		if (it!=m_mapSecurityIDs.end())
		{
			_setInstrumentIDs = it->second;
		}

		_setInstrumentIDs.insert(pSpecificSecurity->SecurityID);
		m_mapSecurityIDs[pSpecificSecurity->ExchangeID] = _setInstrumentIDs;
	}
}

void CLevel2UserApi::OnRspUnSubLevel2MarketData(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//ģ��ƽ̨��������������ᴥ��
	if(!IsErrorRspInfo(pRspInfo,nRequestID,bIsLast)
		&&pSpecificSecurity)
	{
		CLock cl(&m_csMapSecurityIDs);

		set<string> _setInstrumentIDs;
		map<string,set<string> >::iterator it = m_mapSecurityIDs.find(pSpecificSecurity->ExchangeID);
		if (it!=m_mapSecurityIDs.end())
		{
			_setInstrumentIDs = it->second;
		}
		_setInstrumentIDs.erase(pSpecificSecurity->SecurityID);
		m_mapSecurityIDs[pSpecificSecurity->ExchangeID] = _setInstrumentIDs;
	}
}

void CLevel2UserApi::OnRtnLevel2MarketData(CThostFtdcLevel2MarketDataField *pLevel2MarketData)
{
	if(m_msgQueue)
		m_msgQueue->Input_OnRtnLevel2MarketData(this,pLevel2MarketData);
}

void CLevel2UserApi::SubscribeNGTSIndex(const string& szInstrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	vector<char*> vct;

	size_t len = szInstrumentIDs.length()+1;
	char* buf = new char[len];
	strncpy(buf,szInstrumentIDs.c_str(),len);

	len = szExchageID.length()+1;
	char* pExchageID = new char[len];
	strncpy(pExchageID,szExchageID.c_str(),len);

	CLock cl(&m_csMapIndexIDs);

	set<string> _setInstrumentIDs;
	map<string,set<string> >::iterator it = m_mapIndexIDs.find(szExchageID);
	if (it!=m_mapIndexIDs.end())
	{
		_setInstrumentIDs = it->second;
	}

	char* token = strtok(buf, _QUANTBOXC2LEVEL2_SEPS_);
	while(token)
	{
		size_t l = strlen(token);
		if (l>0)
		{
			//��Լ�Ѿ����ڣ������ٶ��ģ�����ζ���Ҳû��ϵ
			_setInstrumentIDs.insert(token);//��¼�˺�Լ���ж���
			vct.push_back(token);
		}
		token = strtok( NULL, _QUANTBOXC2LEVEL2_SEPS_);
	}

	m_mapIndexIDs[szExchageID] = _setInstrumentIDs;
	
	if(vct.size()>0)
	{
		//ת���ַ�������
		CThostFtdcSpecificSecurityField* pArray = new CThostFtdcSpecificSecurityField[vct.size()];
		for (size_t j = 0; j<vct.size(); ++j)
		{
			strncpy(pArray[j].SecurityID,vct[j],sizeof(TThostFtdcSecurityIDType));
			strncpy(pArray[j].ExchangeID,pExchageID,sizeof(TThostFtdcSecurityIDType));
		}

		//����
		m_pApi->SubscribeNGTSIndex(pArray,(int)vct.size());

		delete[] pArray;
	}

	//�ͷ��ڴ�
	delete[] buf;
	delete[] pExchageID;
}

void CLevel2UserApi::UnSubscribeNGTSIndex(const string& szInstrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	vector<char*> vct;

	size_t len = szInstrumentIDs.length()+1;
	char* buf = new char[len];
	strncpy(buf,szInstrumentIDs.c_str(),len);

	len = szExchageID.length()+1;
	char* pExchageID = new char[len];
	strncpy(pExchageID,szExchageID.c_str(),len);

	CLock cl(&m_csMapIndexIDs);

	set<string> _setInstrumentIDs;
	map<string,set<string> >::iterator it = m_mapIndexIDs.find(szExchageID);
	if (it!=m_mapIndexIDs.end())
	{
		_setInstrumentIDs = it->second;
	}

	char* token = strtok(buf, _QUANTBOXC2LEVEL2_SEPS_);
	while(token)
	{
		size_t l = strlen(token);
		if (l>0)
		{
			//��Լ�Ѿ������ڣ�������ȡ�����ģ������ȡ��Ҳûʲô��ϵ
			_setInstrumentIDs.erase(token);//��¼�˺�Լ���ж���
			vct.push_back(token);
		}
		token = strtok( NULL, _QUANTBOXC2LEVEL2_SEPS_);
	}

	m_mapIndexIDs[szExchageID] = _setInstrumentIDs;
	
	if(vct.size()>0)
	{
		//ת���ַ�������
		CThostFtdcSpecificSecurityField* pArray = new CThostFtdcSpecificSecurityField[vct.size()];
		for (size_t j = 0; j<vct.size(); ++j)
		{
			strncpy(pArray[j].SecurityID,vct[j],sizeof(TThostFtdcSecurityIDType));
			strncpy(pArray[j].ExchangeID,pExchageID,sizeof(TThostFtdcSecurityIDType));
		}

		//����
		m_pApi->UnSubscribeNGTSIndex(pArray,(int)vct.size());

		delete[] pArray;
	}

	//�ͷ��ڴ�
	delete[] buf;
	delete[] pExchageID;
}

void CLevel2UserApi::SubscribeNGTSIndex(const set<string>& instrumentIDs,const string& szExchageID)
{
	if(NULL == m_pApi)
		return;

	string szInstrumentIDs;
	for(set<string>::iterator i=instrumentIDs.begin();i!=instrumentIDs.end();++i)
	{
		szInstrumentIDs.append(*i);
		szInstrumentIDs.append(";");
	}

	if (szInstrumentIDs.length()>1)
	{
		SubscribeNGTSIndex(szInstrumentIDs,szExchageID);
	}
}

void CLevel2UserApi::OnRspSubNGTSIndex(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//��ģ��ƽ̨��������������ᴥ��������Ҫ�Լ�ά��һ���Ѿ����ĵĺ�Լ�б�
	if(!IsErrorRspInfo(pRspInfo,nRequestID,bIsLast)
		&&pSpecificSecurity)
	{
		CLock cl(&m_csMapIndexIDs);

		set<string> _setInstrumentIDs;
		map<string,set<string> >::iterator it = m_mapIndexIDs.find(pSpecificSecurity->ExchangeID);
		if (it!=m_mapIndexIDs.end())
		{
			_setInstrumentIDs = it->second;
		}

		_setInstrumentIDs.insert(pSpecificSecurity->SecurityID);
		m_mapIndexIDs[pSpecificSecurity->ExchangeID] = _setInstrumentIDs;
	}
}

void CLevel2UserApi::OnRspUnSubNGTSIndex(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//ģ��ƽ̨��������������ᴥ��
	if(!IsErrorRspInfo(pRspInfo,nRequestID,bIsLast)
		&&pSpecificSecurity)
	{
		CLock cl(&m_csMapIndexIDs);

		set<string> _setInstrumentIDs;
		map<string,set<string> >::iterator it = m_mapIndexIDs.find(pSpecificSecurity->ExchangeID);
		if (it!=m_mapIndexIDs.end())
		{
			_setInstrumentIDs = it->second;
		}
		_setInstrumentIDs.erase(pSpecificSecurity->SecurityID);
		m_mapIndexIDs[pSpecificSecurity->ExchangeID] = _setInstrumentIDs;
	}
}

void CLevel2UserApi::OnRtnNGTSIndex(CThostFtdcNGTSIndexField *pNGTSIndex)
{
	if(m_msgQueue)
		m_msgQueue->Input_OnRtnNGTSIndex(this,pNGTSIndex);
}