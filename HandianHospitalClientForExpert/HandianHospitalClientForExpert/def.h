#pragma once

//����
const TCHAR STR_FILE_FILTER[] =
_T("Picture Files(*.jpg,*.png)\0*.jpg;*.png;\0");

typedef struct _URL_INFO
{
	WCHAR szScheme[512];
	WCHAR szHostName[512];
	WCHAR szUserName[512];
	WCHAR szPassword[512];
	WCHAR szUrlPath[512];
	WCHAR szExtraInfo[512];
}URL_INFO, *PURL_INFO;

//extern CStringW g_zhumu_user;
//extern CStringW g_zhumu_pwd;
//extern  CStringW g_zhumu_meeting_num;
//extern CStringW g_zhumu_key;
//extern CStringW g_zhumu_secret;