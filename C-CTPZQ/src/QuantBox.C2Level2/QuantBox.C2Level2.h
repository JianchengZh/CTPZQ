// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� QUANTBOXC2CTP_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// QUANTBOXC2CTP_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifndef _QUANTBOXC2LEVEL2_H_
#define _QUANTBOXC2LEVEL2_H_

#ifdef QUANTBOXC2LEVEL2_EXPORTS
#define QUANTBOXC2LEVEL2_API __declspec(dllexport)
#else
#define QUANTBOXC2LEVEL2_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "include\Level2\ThostFtdcLevel2UserApiStruct.h"

_USING_LEVEL2_NS_


//���ڷָ�����ĺ��б���ǰ�û���ַ�б����Բ��ܳ��֡�:��һ���
#define _QUANTBOXC2LEVEL2_SEPS_ ",;"

//����״̬ö��
enum ConnectionStatus
{
	E_uninit,		//δ��ʼ��
	E_inited,		//�Ѿ���ʼ��
	E_unconnected,	//�����Ѿ��Ͽ�
	E_connecting,	//������
	E_connected,	//���ӳɹ�
	E_authing,		//��Ȩ��
	E_authed,		//��Ȩ�ɹ�
	E_logining,		//��¼��
	E_logined,		//��¼�ɹ�
	E_confirming,	//���㵥ȷ����
	E_confirmed,	//�Ѿ�ȷ��
	E_conn_max		//���ֵ
};

//�ص��������Ͷ��壨Ϊ��д���㣬����ĸ����
typedef void (__stdcall *fnOnConnect)(void* pApi,CThostFtdcRspUserLoginField *pRspUserLogin,ConnectionStatus result);//���Ӻ�Ľ��״̬
typedef void (__stdcall *fnOnDisconnect)(void* pApi,CThostFtdcRspInfoField *pRspInfo,ConnectionStatus step);//����ʱ������״̬
typedef void (__stdcall *fnOnRspError)(void* pApi,CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
typedef void (__stdcall *fnOnRtnLevel2MarketData)(void* pLevel2Api,CThostFtdcLevel2MarketDataField *pLevel2MarketData);
typedef void (__stdcall *fnOnRtnNGTSIndex)(void* pLevel2Api,CThostFtdcNGTSIndexField *pNGTSIndex);

//����������Ϣ���У�֧����Ӧ����ͽ���
QUANTBOXC2LEVEL2_API void* __stdcall CTP_CreateMsgQueue();

//����Ϣ����ע��ص�����
QUANTBOXC2LEVEL2_API void __stdcall CTP_RegOnConnect(void* pMsgQueue,fnOnConnect pCallback);
QUANTBOXC2LEVEL2_API void __stdcall CTP_RegOnDisconnect(void* pMsgQueue,fnOnDisconnect pCallback);
QUANTBOXC2LEVEL2_API void __stdcall CTP_RegOnRspError(void* pMsgQueue,fnOnRspError pCallback);
QUANTBOXC2LEVEL2_API void __stdcall CTP_RegOnRtnLevel2MarketData(void* pMsgQueue,fnOnRtnLevel2MarketData pCallback);
QUANTBOXC2LEVEL2_API void __stdcall CTP_RegOnRtnNGTSIndex(void* pMsgQueue,fnOnRtnNGTSIndex pCallback);

//�ͷ���Ϣ����
QUANTBOXC2LEVEL2_API void __stdcall CTP_ReleaseMsgQueue(void* pMsgQueue);
//�����Ϣ��������
QUANTBOXC2LEVEL2_API void __stdcall CTP_ClearMsgQueue(void* pMsgQueue);

//������Ϣ���е�һ����¼
QUANTBOXC2LEVEL2_API bool __stdcall CTP_ProcessMsgQueue(void* pMsgQueue);
//�������лص����������߳�
QUANTBOXC2LEVEL2_API void __stdcall CTP_StartMsgQueue(void* pMsgQueue);
//ֹͣ���лص����������߳�
QUANTBOXC2LEVEL2_API void __stdcall CTP_StopMsgQueue(void* pMsgQueue);

//����ӿ�=======================================

//��������ӿ�
QUANTBOXC2LEVEL2_API void* __stdcall L2_CreateLevel2Api();
//����Ϣ����ע�ᵽ����ӿ���
QUANTBOXC2LEVEL2_API void __stdcall L2_RegMsgQueue2Level2Api(void* pLevel2Api,void* pMsgQueue);
//����
QUANTBOXC2LEVEL2_API void __stdcall L2_Connect(
	void* pLevel2Api,
	const char* szPath,
	const char* szAddresses,
	const char* szBrokerId,
	const char* szInvestorId,
	const char* szPassword);

//���ĺ�Լ�������Լ�ԡ�,���ָ�
QUANTBOXC2LEVEL2_API void __stdcall L2_SubscribeLevel2MarketData(void* pLevel2Api,const char* szInstrumentIDs,const char* szExchageID);
QUANTBOXC2LEVEL2_API void __stdcall L2_SubscribeNGTSIndex(void* pLevel2Api,const char* szInstrumentIDs,const char* szExchageID);
//ȡ�����ģ������Լ�ԡ�,���ָ�
QUANTBOXC2LEVEL2_API void __stdcall L2_UnSubscribeLevel2MarketData(void* pLevel2Api,const char* szInstrumentIDs,const char* szExchageID);
QUANTBOXC2LEVEL2_API void __stdcall L2_UnSubscribeNGTSIndex(void* pLevel2Api,const char* szInstrumentIDs,const char* szExchageID);
//�Ͽ�����
QUANTBOXC2LEVEL2_API void __stdcall L2_Disconnect(void* pLevel2Api);
//�ͷ�����ӿ�
QUANTBOXC2LEVEL2_API void __stdcall L2_ReleaseLevel2Api(void* pLevel2Api);


#ifdef __cplusplus
}
#endif


#endif//end of _QUANTBOXC2CTP_H_