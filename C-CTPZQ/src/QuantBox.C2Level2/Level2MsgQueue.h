#pragma once

#include "QuantBox.C2Level2.h"
#include "include\LockFreeQ.h"

class CLevel2MsgQueue
{
	//��Ӧ�����п��ܳ��ֵ���Ϣ���ͣ�����ĸ����
	enum EnumMsgType
	{
		E_fnOnConnect,	
		E_fnOnDisconnect,
		E_fnOnRspError,
		E_fnOnRtnLevel2MarketData,
		E_fnOnRtnNGTSIndex,
	};

	struct SMsgItem
	{
		EnumMsgType							type;			//��Ϣ����
		void*								pApi;			//ָ��������ָ��
		CThostFtdcRspInfoField				RspInfo;		//��Ӧ��Ϣ
		bool								bIsLast;		//�Ƿ����һ����
		union{
			int								nRequestID;
			ConnectionStatus				Status;
		};
		union{
			CThostFtdcLevel2MarketDataField			Level2MarketData;
			CThostFtdcNGTSIndexField				NGTSIndex;
			CThostFtdcRspUserLoginField				RspUserLogin;
		};
	};

public:
	CLevel2MsgQueue(void)
	{
		m_nSleep = 1;
		m_hThread = NULL;
		m_bRunning = false;

		//�ص�������ַָ��
		m_fnOnConnect = NULL;
		m_fnOnDisconnect = NULL;
		m_fnOnRspError = NULL;
		m_fnOnRtnLevel2MarketData = NULL;
		m_fnOnRtnNGTSIndex = NULL;
	}
	virtual ~CLevel2MsgQueue(void)
	{
		StopThread();
		Clear();
	}

public:
	//��ն���
	void Clear();

	//�������ⲿ����˳������д����ص�����
	bool Process();

	//���ڲ������̣߳��ڲ���������Process�����ص�
	void StartThread();
	void StopThread();

	//���ⲿ�ĺ�����ַע�ᵽ����(����ĸ����)
	void RegisterCallback(fnOnConnect pCallback){m_fnOnConnect = pCallback;}
	void RegisterCallback(fnOnDisconnect pCallback){m_fnOnDisconnect = pCallback;}
	void RegisterCallback(fnOnRspError pCallback){m_fnOnRspError = pCallback;}
	void RegisterCallback(fnOnRtnLevel2MarketData pCallback){m_fnOnRtnLevel2MarketData = pCallback;}
	void RegisterCallback(fnOnRtnNGTSIndex pCallback){m_fnOnRtnNGTSIndex = pCallback;}

	//��Ӧ�������������(����ĸ����)
	void Input_OnConnect(void* pApi,CThostFtdcRspUserLoginField *pRspUserLogin,ConnectionStatus result);
	void Input_OnDisconnect(void* pApi,CThostFtdcRspInfoField *pRspInfo,ConnectionStatus step);
	void Input_OnRspError(void* pApi,CThostFtdcRspInfoField* pRspInfo,int nRequestID,bool bIsLast);
	void Input_OnRtnLevel2MarketData(void* pLevel2Api,CThostFtdcLevel2MarketDataField *pLevel2MarketData);
	void Input_OnRtnNGTSIndex(void* pLevel2Api,CThostFtdcNGTSIndexField *pNGTSIndex);
private:
	friend DWORD WINAPI ProcessThread(LPVOID lpParam);
	void RunInThread();

	//��Ӧ���ֱ�������
	void _Input(SMsgItem* pMsgItem);
	//�����е���Ϣ�жϷַ�
	void _Output(SMsgItem* pMsgItem);

	//��Ӧ���
	void Output_OnConnect(SMsgItem* pItem)
	{
		if(m_fnOnConnect)
			(*m_fnOnConnect)(pItem->pApi,&pItem->RspUserLogin,pItem->Status);
	}
	void Output_OnDisconnect(SMsgItem* pItem)
	{
		if(m_fnOnDisconnect)
			(*m_fnOnDisconnect)(pItem->pApi,&pItem->RspInfo,pItem->Status);
	}
	void Output_OnRspError(SMsgItem* pItem)
	{
		if(m_fnOnRspError)
			(*m_fnOnRspError)(pItem->pApi,&pItem->RspInfo,pItem->nRequestID,pItem->bIsLast);
	}
	void Output_OnRtnLevel2MarketData(SMsgItem* pItem)
	{
		if(m_fnOnRtnLevel2MarketData)
			(*m_fnOnRtnLevel2MarketData)(pItem->pApi,&pItem->Level2MarketData);
	}
	void Output_OnRtnNGTSIndex(SMsgItem* pItem)
	{
		if(m_fnOnRtnNGTSIndex)
			(*m_fnOnRtnNGTSIndex)(pItem->pApi,&pItem->NGTSIndex);
	}
private:
	int							m_nSleep;
	bool						m_bRunning;
	HANDLE						m_hThread;

	MSQueue<SMsgItem*>			m_queue;			//��Ӧ����

	//�ص�����ָ�루����ĸ����
	fnOnConnect							m_fnOnConnect;
	fnOnDisconnect						m_fnOnDisconnect;
	fnOnRspError						m_fnOnRspError;
	fnOnRtnLevel2MarketData				m_fnOnRtnLevel2MarketData;
	fnOnRtnNGTSIndex					m_fnOnRtnNGTSIndex;
};

