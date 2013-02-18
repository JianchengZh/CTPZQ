#pragma once

#include "QuantBox.C2Level2.h"
#include "include\Level2\ThostFtdcLevel2UserApi.h"

#include <set>
#include <map>
#include <string>

using namespace std;
_USING_LEVEL2_NS_

class CLevel2MsgQueue;

class CLevel2UserApi:public CThostFtdcLevel2UserSpi
{
public:
	CLevel2UserApi(void);
	virtual ~CLevel2UserApi(void);

	void RegisterMsgQueue(CLevel2MsgQueue* pMsgQueue);

	void Connect(const string& szPath,
		const string& szAddresses,
		const string& szBrokerId,
		const string& szInvestorId,
		const string& szPassword);
	void Disconnect();

	void SubscribeLevel2MarketData(const string& szInstrumentIDs,const string& szExchageID);
	void UnSubscribeLevel2MarketData(const string& szInstrumentIDs,const string& szExchageID);
	void SubscribeNGTSIndex(const string& szInstrumentIDs,const string& szExchageID);
	void UnSubscribeNGTSIndex(const string& szInstrumentIDs,const string& szExchageID);

private:
	//��������
	void SubscribeLevel2MarketData(const set<string>& instrumentIDs,const string& szExchageID);
	void SubscribeNGTSIndex(const set<string>& instrumentIDs,const string& szExchageID);

	//��¼����
	void ReqUserLogin();

	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///Level2����
	virtual void OnRspSubLevel2MarketData(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUnSubLevel2MarketData(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRtnLevel2MarketData(CThostFtdcLevel2MarketDataField *pLevel2MarketData);
	///Level2ָ������
	virtual void OnRspSubNGTSIndex(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUnSubNGTSIndex(CThostFtdcSpecificSecurityField *pSpecificSecurity, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRtnNGTSIndex(CThostFtdcNGTSIndexField *pNGTSIndex);

private:
	//����Ƿ����
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);//��������Ϣ�͵���Ϣ����
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);//���ͳ�����Ϣ

private:
	ConnectionStatus			m_status;				//����״̬
	int							m_nRequestID;			//����ID��ÿ������ǰ����

	map<string,set<string> >	m_mapSecurityIDs;		//���ڶ��ĵĺ�Լ
	map<string,set<string> >	m_mapIndexIDs;			//���ڶ��ĵĺ�Լ

	CThostFtdcLevel2UserApi*	m_pApi;					//����API
	CLevel2MsgQueue*			m_msgQueue;				//��Ϣ����ָ��

	string						m_szPath;				//���������ļ���·��
	set<string>					m_arrAddresses;			//��������ַ
	string						m_szBrokerId;			//����ID
	string						m_szInvestorId;			//Ͷ����ID
	string						m_szPassword;			//����
};

