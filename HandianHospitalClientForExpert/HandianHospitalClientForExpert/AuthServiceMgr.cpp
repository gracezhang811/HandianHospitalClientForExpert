#include "stdafx.h"
#include "AuthServiceMgr.h"
#include "zhumu_sdk_def.h"

CAuthServiceMgr::CAuthServiceMgr()
{
	m_bInited = false;
	m_bLogined = false;
}

CAuthServiceMgr::~CAuthServiceMgr()
{
	m_bInited = false;
	m_bLogined = false;
}

wchar_t* CAuthServiceMgr::AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
	{
		return NULL;
	}
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}

//将宽字节wchar_t*转化为单字节char*  
inline char* CAuthServiceMgr::UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}

bool CAuthServiceMgr::Init(ZHUMU_SDK_NAMESPACE::InitParam* initParam)
{
	if (m_bInited)
		return true;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_InitSDK(initParam) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_CreateAuthService(&m_pAuthService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (m_pAuthService == NULL)
		return false;

	if (m_pAuthService->SetEvent(this) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	m_bInited = true;


	return true;
}

bool CAuthServiceMgr::UnInit()
{
	if (!m_bInited)
		return true;

	m_bInited = false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_DestroyAuthService(m_pAuthService) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	if (ZHUMU_SDK_NAMESPACE::ZHUMU_CleanUPSDK() != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	return true;
}

bool CAuthServiceMgr::SDKAuth(ZHUMU_SDK_NAMESPACE::AuthParam& authParam)
{
	if (m_pAuthService == NULL)
		return false;

	if (m_pAuthService->SDKAuth(authParam) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	return true;
}

bool CAuthServiceMgr::Login(ZHUMU_SDK_NAMESPACE::LoginParam& param)
{
	if (!m_pAuthService)
		return false;

	if (m_pAuthService->Login(param) != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS)
		return false;

	return true;
}

bool CAuthServiceMgr::LogOut()
{
	if (!m_bLogined) {
		return true;
	}		

	if (!m_pAuthService) {
		return false;
	}
		

	if (m_pAuthService->LogOut() != ZHUMU_SDK_NAMESPACE::SDKERR_SUCCESS) {
		m_bLogined = true;
		return false;
	}
	else {		
		return true;
	}
	
}

void CAuthServiceMgr::onAuthenticationReturn(ZHUMU_SDK_NAMESPACE::AuthResult ret)
{
}

void CAuthServiceMgr::onLoginRet(ZHUMU_SDK_NAMESPACE::LOGINSTATUS status, ZHUMU_SDK_NAMESPACE::IAccountInfo* pAccountInfo)
{
	char szLog[MAX_PATH] = { 0 };
	OutputDebugStringA("\n onLoginRet.\n");

	if (ZHUMU_SDK_NAMESPACE::LOGIN_SUCCESS == status)
	{
		OutputDebugStringA("\n Login success.\n");
		m_bLogined = true;

	}
	else
	{
		OutputDebugStringA("\n Login failed.\n");
	}

}


void CAuthServiceMgr::onLogout()
{
	m_bLogined = false;
	OutputDebugStringA("\n onLoginOutRet.\n");
}