#include "stdafx.h"
#include "HandianRemoteTreatment.h"
#include "AuthServiceMgr.h"
#include "MeetingServiceMgr.h"

CStringA token("");
CStringA inmeetingappointmentno("");
CStringA editrecordappointmentno("");
CStringW rtmessage("");
CStringW g_zhumu_user = _T("");
CStringW g_zhumu_pwd = _T("");
CStringW g_zhumu_meeting_num = _T("");
//CStringW g_zhumu_key = _T("");
//CStringW g_zhumu_secret = _T("");
//int inmeeting = 0;
int meetingfailed = 0;
int inplaying = 0;
int timer1used = 0;
int timer2used = 0;

int appointlisttotalpagenum = 1;
int appointlistcurrentpagenum = 1;
CStringW appointsearchtext = L"";
int recordlisttotalpagenum = 1;
int recordlistcurrentpagenum = 1;
CStringW recordsearchtext = L"";

HANDLE hFile;
char logbuf[30];

DUI_BEGIN_MESSAGE_MAP(CDuiFrameWnd, WindowImplBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMBUTTONCLICK, OnItemButtonClick)
DUI_END_MESSAGE_MAP()

CDuiFrameWnd::CDuiFrameWnd()
{
	ResetAllControls();
	m_pAuthServiceMgr = new CAuthServiceMgr();
	m_pMeetingServiceMgr = new CMeetingServiceMgr();
	m_pAVPlayer = new CAVPlayer();
	m_bSDKInit = false;
	m_bZhumuAuth = false;
}

CDuiFrameWnd::~CDuiFrameWnd() {
	if (m_pMeetingServiceMgr)
	{
		delete m_pMeetingServiceMgr;
		m_pMeetingServiceMgr = NULL;
	}
	if (m_pAuthServiceMgr)
	{
		delete m_pAuthServiceMgr;
		m_pAuthServiceMgr = NULL;
	}
	if (m_pAVPlayer)
	{
		delete m_pAVPlayer;
		m_pAVPlayer = NULL;
	}
	m_bSDKInit = false;
	m_bZhumuAuth = false;
};

void CDuiFrameWnd::HDWriteLog(CStringA logmsg)
{
	DWORD dwWritenSize = 0;
	BOOL bRet = WriteFile(hFile, logmsg, logmsg.GetLength(), &dwWritenSize, NULL);
	if (!bRet)
	{
		OutputDebugStringA("WriteFile 写文件失败\r\n");
	}
	//刷新文件缓冲区  
	FlushFileBuffers(hFile);
}

bool CDuiFrameWnd::SDKAuth()
{
	std::wstring strWebDomain = TEST_ZOOM_WEBDOMAIN;

	if (!m_bSDKInit)
	{
		ZHUMU_SDK_NAMESPACE::InitParam initParam;
		initParam.strWebDomain = strWebDomain.c_str();
		if (m_pAuthServiceMgr)
			m_bSDKInit = m_pAuthServiceMgr->Init(&initParam);
			if (!m_bSDKInit)
			{
				OutputDebugStringW(L"sdk init failed\n");
				return false;
			}
			else {
				OutputDebugStringW(L"sdk init success\n");
				ZHUMU_SDK_NAMESPACE::AuthParam authParam;
				//CStringW key = g_zhumu_key;
				//CStringW secret = g_zhumu_secret;
				authParam.appKey = TEST_ZOOM_KEY;
				authParam.appSecret = TEST_ZOOM_SECRET;
				return m_pAuthServiceMgr->SDKAuth(authParam);
			}
	}
	else {
		return false;
	}
}

bool CDuiFrameWnd::SDKLogin()
{
	ZHUMU_SDK_NAMESPACE::LoginParam param;
	param.userName = g_zhumu_user;
	param.password = g_zhumu_pwd;
	param.bRememberMe = true;
	return m_pAuthServiceMgr->Login(param);
}

void CDuiFrameWnd::SDKStartMeeting() {
	if (!m_pMeetingServiceMgr->Init())
	{
		OutputDebugStringA("MeetingServiceInit失败！\n");
		CStringA logmsg = "会议初始化失败。\r\n";
		HDWriteLog(logmsg);
		m_labWaitMeetingStart->SetVisible(false);
		meetingfailed = 1;
		return;
	}
	else {
		OutputDebugStringA("MeetingServiceInit成功\n");
		CStringA logmsg = "会议初始化成功。\r\n";
		HDWriteLog(logmsg);
		ZHUMU_SDK_NAMESPACE::StartParam startParam;
		startParam.userType = ZHUMU_SDK_NAMESPACE::SDK_UT_NORMALUSER;
		ZHUMU_SDK_NAMESPACE::StartParam4NormalUser& normalParam = startParam.param.normaluserStart;
		
		std::wstring strMeetingNumber = g_zhumu_meeting_num;
		//normalParam.meetingNumber = g_zhumu_meeting_num;
		normalParam.meetingNumber = _wtoi64(strMeetingNumber.c_str());;
		normalParam.isDirectShareDesktop = false;///direct share desktop or not when you start meeting
		normalParam.hDirectShareAppWnd = NULL;///direct share window or not when you start meeting
		bool bStart = m_pMeetingServiceMgr->Start(startParam);

		if (!bStart)
		{
			OutputDebugStringA("开始会议失败！！\n");
			CStringA logmsg = "开始会议失败。\r\n";
			HDWriteLog(logmsg);
			m_labWaitMeetingStart->SetVisible(false);
			meetingfailed = 1;
		}
		else
		{
			OutputDebugStringA("开始会议！\n");
			CStringA logmsg = "开始会议成功。\r\n";
			HDWriteLog(logmsg);
			m_pMeetingServiceMgr->appointment_num = inmeetingappointmentno;
			m_pMeetingServiceMgr->token1 = token;
			//inmeeting = 1;		
			meetingfailed = 0;
			m_labWaitMeetingStart->SetVisible(false);
		}
	}
}


int CDuiFrameWnd::HttpLogin() {
	LPCTSTR curl = LOGIN_URL;
	//CStringA postdata = "username=13111111111&password=111111&grant_type=password&client_id=testclient&client_secret=testpass&type=2";
	CStringA postdata = "";
	postdata += "username=";
	//postdata += "damen";
	postdata += m_editMobileOnLoginUI->GetText();
	postdata += "&password=";
	//postdata += "111111";
	postdata += m_editPWDOnLoginUI->GetText();
	postdata += "&grant_type=";
	postdata += GRANT_TYPE;
	postdata += "&client_id=";
	postdata += CLIENT_ID;
	postdata += "&client_secret=";
	postdata += CLIENT_SECRET;
	postdata += "&type=";
	postdata += LOGIN_TYPE;
	CStringW temp = L"";

	Json::Value root = HDWinHttpPost(curl, postdata);

	if (root["code"] == 0) {
		OutputDebugStringA("login success \n ");
		token = root["data"]["access_token"].asCString();
		int userstatus = HttpGetUserInfo();
		if (userstatus == 1) {
			return 1;
		}
		else {
			::MessageBox(*this, _T("登录失败，请检查用户名和密码！"), _T("提示"), 0);
			return -1;
		}
	}
	else if (root["code"] == -1) {
		::MessageBox(*this, _T("网络错误！"), _T("提示"), 0);
		return -1;
	}
	else {
		::MessageBox(*this, _T("登录失败，请检查用户名和密码！"), _T("提示"), 0);
		return -1;
	}

}


void CDuiFrameWnd::HttpGetAppointmentDetail(LPCTSTR appointmentnumber) {
	CStringW curl = APPOINTMENT_DETAIL_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get appointment info success \n ");
		temp = UTF8ToWString(root["data"]["clinic"]["name"].asCString());
		m_PaintManager.FindControl(_T("LabalClinicOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["order_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabalStartTimeOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["order_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabalEndTimeOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabalPatientOnAMDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabalIllnessInfoOnAMDetail"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1"].asCString());
			m_btnShowPatientPic1UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic1UI->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					OutputDebugStringA("download success.\n");
					m_btnShowPatientPic1UI->SetBkImage(path1);
				}
				else {
					m_btnShowPatientPic1UI->SetBkImage(L"bkimage/downloadimagefailed.png");
				}
			}	
		}
		else {
			m_btnShowPatientPic1UI->SetVisible(false);
		}

		if ((root["data"]["patient_img2_thumb"] != "")&&(root["data"]["patient_img2_thumb"].isNull() == false )){
			temp = UTF8ToWString(root["data"]["patient_img2"].asCString());
			m_btnShowPatientPic2UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic2UI->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					OutputDebugStringA("download success.\n");
					m_btnShowPatientPic2UI->SetBkImage(path2);
				}
				else {
					m_btnShowPatientPic2UI->SetBkImage(L"bkimage/downloadimagefailed.png");
				}
			}		
		}
		else {
			m_btnShowPatientPic2UI->SetVisible(false);
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3"].asCString());
			m_btnShowPatientPic3UI->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic3UI->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_btnShowPatientPic3UI->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic3UI->SetBkImage(L"bkimage/downloadimagefailed.png");
				}
			}						
		}
		else {
			m_btnShowPatientPic3UI->SetVisible(false);
		}

		int cost = root["data"]["order_fee"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", cost);
		CStringW thecost = aa;
		m_PaintManager.FindControl(_T("LabalCostFeeOnAMDetail"))->SetText(thecost);

	}
	else {
		OutputDebugStringA("get appointment info failed \n ");
	}
}


int CDuiFrameWnd::HttpGetAppointmentStatus(LPCTSTR appointmentnumber) {
	CStringW curl = APPOINTMENT_DETAIL_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		int dxstatus = root["data"]["dx_status"].asInt();
		return dxstatus;
	}
	else {
		OutputDebugStringA("get appointment info failed \n ");
		return -1;
	}
}


int CDuiFrameWnd::HttpSendCode(LPCTSTR mobile) {
	CStringW curl = SEND_CODE_URL;
	curl += L"?mobile=";
	curl += mobile;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGet(curl);

	if (root["code"] == 0) {
		OutputDebugStringA("send code success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("send code failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}



int CDuiFrameWnd::HttpResetPassword(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPost(RESET_PASSWORD_URL, poststr);

	if (root["code"] == 0) {
		OutputDebugStringA("reset pwd success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("reset pwd failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}




void CDuiFrameWnd::HttpGetAppointmentList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get appointment list success \n ");
		int size = root["data"].size();
		CStringW temp;

		appointlisttotalpagenum = root["totalPage"].asInt();
		appointlistcurrentpagenum = pagenum;
		int totalpagenum1 = root["totalPage"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", totalpagenum1);
		CStringW totalpagenum2 = aa;
		m_labPageNumOnAppoint->SetText(totalpagenum2);

		sprintf_s(aa, "%d", pagenum);
		CStringW cpagenum = aa;
		m_labCurrentPageNumOnAppoint->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				//char aa[20];
				sprintf_s(aa, "%llu", appnum);
				CStringW appnostr = aa;
				int dxstatus = root["data"][i]["dx_status"].asInt();

				CDialogBuilder builder;
				CListContainerElementUI* pLine;
				pLine = (CListContainerElementUI*)(builder.Create(_T("AppointmentLine.xml"), (UINT)0, this));
				if (pLine != NULL)
				{
					pLine->SetUserData(appnostr);
					pLine->SetTag(dxstatus);
					m_listAppointmentUI->Add(pLine);
					CLabelUI* pAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnAMLine"));
					temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
					pAppointmentTime->SetText(temp);

					CLabelUI* pAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentClinicOnAMLine"));
					temp = UTF8ToWString(root["data"][i]["clinic"]["name"].asCString());
					pAppointmentDoctor->SetText(temp);

					CLabelUI* pAppointmentPatient = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentPatientOnAMLine"));
					temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
					pAppointmentPatient->SetText(temp);

				}
	
			}
		}

	}
	else {
		OutputDebugStringA("get appointment list failed \n ");
	}
}


void CDuiFrameWnd::HttpGetCurrentAppointmentList(LPCTSTR curl) {
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get current appointment list success \n ");
		int size = root["data"].size();
		CStringW temp;
		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				char aa[20];
				sprintf_s(aa, "%llu", appnum);
				CStringW appnostr = aa;
				int dxstatus = root["data"][i]["dx_status"].asInt();
				if (dxstatus == 1) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("CurrentConsultationLine.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						pLine->SetTag(dxstatus);
						m_listCAppointmentUI->Add(pLine);

						CLabelUI* pCAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pCAppointmentTime->SetText(temp);

						CLabelUI* pCAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentClinicOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["clinic"]["name"].asCString());
						pCAppointmentDoctor->SetText(temp);

					}
				}
				else if (dxstatus == 2) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("CurrentConsultationLine_Done.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						pLine->SetTag(dxstatus);
						m_listCAppointmentUI->Add(pLine);

						CLabelUI* pCAppointmentTime = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pCAppointmentTime->SetText(temp);

						CLabelUI* pCAppointmentDoctor = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentClinicOnCAMLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pCAppointmentDoctor->SetText(temp);

					}
				}

			}
		}

	}
	else {
		OutputDebugStringA("get current appointment list failed \n ");
	}

}


void CDuiFrameWnd::HttpGetRecordList(LPCTSTR curl, int pagenum) {
	CStringW pagecount;
	pagecount.Format(_T("%d"), pagenum);
	CStringW url = curl;
	url += L"&page=";
	url += pagecount;
	OutputDebugStringW(url);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(url, token);

	if (root["code"] == 0) {
		OutputDebugStringA("get record list success \n ");
		int size = root["data"].size();
		CStringW temp;

		recordlisttotalpagenum = root["totalPage"].asInt();
		recordlistcurrentpagenum = pagenum;

		int totalpagenum1 = root["totalPage"].asInt();
		char aa[20];
		sprintf_s(aa, "%d", totalpagenum1);
		CStringW totalpagenum2 = aa;
		m_labPageNumOnRecord->SetText(totalpagenum2);

		sprintf_s(aa, "%d", pagenum);
		CStringW cpagenum = aa;
		m_labCurrentPageNumOnRecord->SetText(cpagenum);

		if (size >= 1) {
			for (int i = 0; i < size; ++i) {
				unsigned __int64 appnum = root["data"][i]["appointment_no"].asInt64();
				//char aa[20];
				sprintf_s(aa, "%llu", appnum);
				CStringW appnostr = aa;
				if (root["data"][i]["change_status"].asInt() == 1) {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("ConsultationRecordLine.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listRecordUI->Add(pLine);
						CLabelUI* pAppointmentTime1 = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pAppointmentTime1->SetText(temp);

						CLabelUI* pRecordClinicName = (CLabelUI*)pLine->FindSubControl(_T("LabelClinicNameOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["clinict_name"].asCString());
						pRecordClinicName->SetText(temp);

						CLabelUI* pRecordPatientName = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pRecordPatientName->SetText(temp);

					}
				}
				else {
					CDialogBuilder builder;
					CListContainerElementUI* pLine;
					pLine = (CListContainerElementUI*)(builder.Create(_T("ConsultationRecordLine_Done.xml"), (UINT)0, this));
					if (pLine != NULL)
					{
						pLine->SetUserData(appnostr);
						m_listRecordUI->Add(pLine);
						CLabelUI* pAppointmentTime1 = (CLabelUI*)pLine->FindSubControl(_T("LabelAppointmentTimeOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["order_starttime"].asCString());
						pAppointmentTime1->SetText(temp);

						CLabelUI* pRecordClinicName = (CLabelUI*)pLine->FindSubControl(_T("LabelClinicNameOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["clinict_name"].asCString());
						pRecordClinicName->SetText(temp);

						CLabelUI* pRecordPatientName = (CLabelUI*)pLine->FindSubControl(_T("LabelPatientNameOnRecordLine"));
						temp = UTF8ToWString(root["data"][i]["patient_name"].asCString());
						pRecordPatientName->SetText(temp);

					}
				}
			}
		}
	}
	else {
		OutputDebugStringA("get record list failed \n ");
	}
}


void CDuiFrameWnd::HttpGetRecordDetail(LPCTSTR appointmentnumber) {
	CStringW curl = RECORD_DETAIL_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info success \n ");
		temp = UTF8ToWString(root["data"]["clinic_name"].asCString());
		m_PaintManager.FindControl(_T("LabelClinicNameOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnRecordDetail"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescNameOnRecordDetail"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1"].asCString());
			m_btnShowPatientPic1UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path1 += L"-pic1-";
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic1UIOnRDDetail->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_btnShowPatientPic1UIOnRDDetail->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic1UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic1UIOnRDDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2"].asCString());
			m_btnShowPatientPic2UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += L"-pic2-";
			//int index = temp.ReverseFind('/');
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic2UIOnRDDetail->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_btnShowPatientPic2UIOnRDDetail->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic2UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic2UIOnRDDetail->SetVisible(false);
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3"].asCString());
			m_btnShowPatientPic3UIOnRDDetail->SetUserData(temp);
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			path3 += L"-pic3-";
			//int index = temp.ReverseFind('/');
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_btnShowPatientPic3UIOnRDDetail->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_btnShowPatientPic3UIOnRDDetail->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_btnShowPatientPic3UIOnRDDetail->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}
		else {
			m_btnShowPatientPic3UIOnRDDetail->SetVisible(false);
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnRecordDetail"))->SetText(temp);

		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordDetail"))->SetText(bb);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordDetail"))->SetText(L"");
		}


		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 2) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordDetail"))->SetText(_T("女"));
		}
		else if (sex == 1) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordDetail"))->SetText(_T("男"));
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordDetail"))->SetText(L"");
		}

		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordDetail"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelExpertAdviseOnRecordDetail"))->SetText(L"");
		}

		CStringW mp3url = root["data"]["audio_url"].asCString();
		m_PaintManager.FindControl(_T("ButtonPlayMp3Record"))->SetUserData(mp3url);
	}
	else {
		OutputDebugStringA("get record info failed \n ");
	}

}


void CDuiFrameWnd::HttpGetRecordUpdate(LPCTSTR appointmentnumber) {
	CStringW curl = USER_UPDATE_RECORD_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info update success \n ");

		unsigned __int64 appnum = root["data"]["appointment_no"].asInt64();
		char aa[20];
		sprintf_s(aa, "%llu", appnum);
		editrecordappointmentno = aa;

		temp = UTF8ToWString(root["data"]["clinic_name"].asCString());
		m_PaintManager.FindControl(_T("LabelClinicNameOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnRecordEdit"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescOnRecordEdit"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += L"-pic1-";
			//int index = temp.ReverseFind('/');
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}


		if ((root["data"]["patient_img2_thumb"] != "") && (root["data"]["patient_img2_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img2_thumb"].asCString());
			CStringW path2 = L"..\\hdtemp\\";
			path2 += appointmentnumber;
			path2 += L"-pic2-";
			//int index = temp.ReverseFind('/');
			path2 += temp.Mid(60);
			if (PathFileExists(path2) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		if ((root["data"]["patient_img3_thumb"] != "") && (root["data"]["patient_img3_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img3_thumb"].asCString());
			CStringW path3 = L"..\\hdtemp\\";
			path3 += appointmentnumber;
			//int index = temp.ReverseFind('/');
			path3 += L"-pic3-";
			path3 += temp.Mid(60);
			if (PathFileExists(path3) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnRecordEdit"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnRecordEdit"))->SetText(temp);

		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordEdit"))->SetText(bb);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientAgeOnRecordEdit"))->SetText(L"");
		}

		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 2) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordEdit"))->SetText(_T("女"));
		}
		else if (sex == 1) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnRecordEdit"))->SetText(_T("男"));
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelPatientIDOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnRecordEdit"))->SetText(L"");
		}

		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("EditExpertAdviseOnRecordEdit"))->SetText(temp);
		}
		else {
			m_PaintManager.FindControl(_T("EditExpertAdviseOnRecordEdit"))->SetText(L"");
		}

	}
	else {
		OutputDebugStringA("get record info update failed \n ");
	}
}



int CDuiFrameWnd::HttpUpdateRecord(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_RECORD_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update record success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update record failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}





int CDuiFrameWnd::HttpGetUserInfo() {
	Json::Value root = HDWinHttpGetWithToken(USER_INFO_URL, token);
	if (root["code"] == 0) {
		OutputDebugStringA("get user info success \n ");
		if (root["data"]["expert"].isNull()) {
			return 3;
		}
		else {
			CStringW temp = UTF8ToWString(root["data"]["expert"]["name"].asCString());
			m_labTitleExpertNameUI->SetText(temp);
			return 1;
		}

	}
	else {
		OutputDebugStringA("get user info failed \n ");
		return -1;
	}
}

int CDuiFrameWnd::HttpCheckSendStartMsg(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(SEND_START_MSG_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("send start message success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("send start message  failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}


void CDuiFrameWnd::HttpSendStartMsg(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(SEND_START_MSG_URL, poststr, token);
}

int CDuiFrameWnd::HttpCreateZhumuMeeting(CStringA poststr) {
	Json::Value root = HDWinHttpPostWithToken(CREATE_ZHUMU_MEETING, poststr, token);
	if (root["code"] == 0) {
		HDWriteLog(logbuf);
		OutputDebugStringA("create zhumu meeting success \n ");

		m_labWaitMeetingStart->SetVisible(true);
		g_zhumu_user = UTF8ToWString(root["data"]["username"].asCString());
		g_zhumu_pwd = UTF8ToWString(root["data"]["password"].asCString());
		g_zhumu_meeting_num = UTF8ToWString(root["data"]["meeting_number"].asCString());
		//g_zhumu_key = UTF8ToWString(root["data"]["app_key"].asCString());
		//g_zhumu_secret = UTF8ToWString(root["data"]["app_secret"].asCString());
		if (m_pAuthServiceMgr->m_bLogined == true) {
			OutputDebugStringW(L"is in login.\r\n");
			if (m_pAuthServiceMgr->LogOut() == false) {
				m_labWaitMeetingStart->SetVisible(false);
				OutputDebugStringW(L"log out sdk failed.\r\n");
				return -1;
			}
			else {
				OutputDebugStringW(L"log out sdk sucess.\r\n");
				SetTimer(*this, 2,2000, NULL);
				timer2used = 1;
				return -1;
			}
		}
		else {
			if (SDKLogin() == true) {
				OutputDebugStringW(L"sdk login success.\n");
				CStringA logmsg = "会议登录成功。\r\n";
				HDWriteLog(logmsg);
				return 1;
			}
			else {
				OutputDebugStringW(L"sdk login failed.\n");
				m_labWaitMeetingStart->SetVisible(false);
				CStringA logmsg = "会议登录失败。\r\n";
				HDWriteLog(logmsg);
				rtmessage = L"视频会议登录失败！";
				return -1;
			}
		}	
	}
	else {
		OutputDebugStringA("create zhumu meeting failed \n ");
		rtmessage = L"会诊通信服务器繁忙，请稍后再试！";
		return -1;
	}
}

/*
void CDuiFrameWnd::HttpSendZhumuEndMesg(CStringA poststr) {
	OutputDebugStringA(poststr);
	OutputDebugStringA("\r\n");
	Json::Value root = HDWinHttpPostWithToken(SEND_ZHUMU_MEETING_END_MESG, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("send zhumu end msg success \n ");
	}
	else {
		OutputDebugStringA("send zhumu end msg failed \n ");
		rtmessage = UTF8ToWString(root["message"].asCString());
	}
}
*/

void CDuiFrameWnd::InitAllControls() {
	m_containerTitleUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_toptitle")));
	m_containerMainpageUI
		= static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_mainpage")));
	m_containerLoginUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_login")));
	m_containerResetPwdUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_resetpwd")));

	m_containerAppointmentListUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_appointmentlist")));
	m_containerAppointmentDetailUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_appointmentdetail")));

	m_containerRecordListUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_recordlist")));
	m_containerRecordDetailUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_recorddetail")));
	m_containerEditRecordUI = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("container_editrecord")));

	m_editMobileOnLoginUI = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditMobileOnLogin")));
	m_editPWDOnLoginUI = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("EditPWDOnLogin")));

	m_btnCloseUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonClose")));
	m_btnCloseEXEUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonCloseEXE")));
	m_btnWndMinUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));

	m_btnLoginUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonLogin")));
	m_btnToResetPwdUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonToResetPwd")));
	m_btnGetResetVerificaionUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonSendResetVerificationOnResetPWD")));
	m_btnResetPwdUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonResetPwd")));

	m_tabMainControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchOperationType")));
	m_tabPatientInfoOnDetailControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnDetail")));
	m_tabPatientInfoOnRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnRecord")));
	m_tabPatientInfoOnEditRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnEditRecord")));

	m_labTitleExpertNameUI = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelTitleExpertName")));
	m_labTitleExpertImageUI = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelTitleExpertImage")));

	m_dataChooseOnSearchAppoint = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnSearchAppoint")));
	m_dataChooseOnSearchRecord = static_cast<CDateTimeUI*>(m_PaintManager.FindControl(_T("DateChooseOnSearchRecord")));

	m_listAppointmentUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListAppointments")));	
	m_listCAppointmentUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListCAppointment")));
	m_listRecordUI = static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListRecord")));

	m_optionCheckAppointment = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CheckAppointment")));
	m_optionCurrentConsultation = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("CurrentConsultation")));
	m_optionConsultationRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("ConsultationRecord")));

	m_optionPatientConditionPageOnShowAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnDetail")));
	m_optionPatientPicPageOnShowAM = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnDetail")));

	m_optionPatientConditionPageOnShowRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnRecord")));
	m_optionPatientPicPageOnShowRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnRecord")));
	m_optionPatientConditionPageOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnEditRecord")));
	m_optionPatientPicPageOnEditRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnEditRecord")));

	m_btnShowPatientPic1UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic1")));
	m_btnShowPatientPic2UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic2")));
	m_btnShowPatientPic3UI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic3")));

	m_btnShowPatientPic1UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic1OnRecord")));
	m_btnShowPatientPic2UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic2OnRecord")));
	m_btnShowPatientPic3UIOnRDDetail = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonShowPatientPic3OnRecord")));

	m_btnPlayAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonPlayMp3Record")));
	m_btnPauseAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonPauseMp3Record")));
	m_btnStopAudio = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonStopMp3Record")));

	m_btnReturnToAppointmentListUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonReturnToAppointmentList")));

	m_labWaitMeetingStart = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelWaitMeetingStart")));

	m_labPageNumOnAppoint = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnAppoint")));
	m_labCurrentPageNumOnAppoint = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnAppoint")));
	m_labPageNumOnRecord = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelPageNumOnRecord")));
	m_labCurrentPageNumOnRecord = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("LabelCurrentPageNumOnRecord")));

}

void CDuiFrameWnd::InitWindow() {
	InitAllControls();
};


void CDuiFrameWnd::ResetAllControls() {
	m_containerTitleUI = NULL;
	m_containerMainpageUI = NULL;
	m_containerLoginUI = NULL;
	m_containerResetPwdUI = NULL;

	m_containerAppointmentListUI = NULL;
	m_containerAppointmentDetailUI = NULL;

	m_containerRecordListUI = NULL;
	m_containerRecordDetailUI = NULL;
	m_containerEditRecordUI = NULL;

	m_editMobileOnLoginUI = NULL;
	m_editPWDOnLoginUI = NULL;

	m_btnCloseUI = NULL;
	m_btnCloseEXEUI = NULL;
	m_btnWndMinUI = NULL;

	m_btnLoginUI = NULL;
	m_btnToResetPwdUI = NULL;
	m_btnGetResetVerificaionUI = NULL;
	m_btnResetPwdUI = NULL;

	m_labTitleExpertNameUI = NULL;
	m_labTitleExpertImageUI = NULL;

	m_tabMainControlUI = NULL;
	m_tabPatientInfoOnDetailControlUI = NULL;
	m_tabPatientInfoOnRecordControlUI = NULL;
	m_tabPatientInfoOnEditRecordControlUI = NULL;

	m_dataChooseOnSearchAppoint = NULL;
	m_dataChooseOnSearchRecord = NULL;

	m_listAppointmentUI = NULL;
	m_listCAppointmentUI = NULL;
	m_listRecordUI = NULL;

	m_btnShowPatientPic1UI = NULL;
	m_btnShowPatientPic2UI = NULL;
	m_btnShowPatientPic3UI = NULL;
	m_btnReturnToAppointmentListUI = NULL;

	m_btnShowPatientPic1UIOnRDDetail = NULL;
	m_btnShowPatientPic2UIOnRDDetail = NULL;
	m_btnShowPatientPic3UIOnRDDetail = NULL;

	m_btnPlayAudio = NULL;
	m_btnPauseAudio = NULL;
	m_btnStopAudio = NULL;

	m_optionCheckAppointment = NULL;
	m_optionCurrentConsultation = NULL;
	m_optionConsultationRecord = NULL;

	m_optionPatientConditionPageOnShowAM = NULL;
	m_optionPatientPicPageOnShowAM = NULL;

	m_optionPatientConditionPageOnShowRecord = NULL;
	m_optionPatientPicPageOnShowRecord = NULL;
	m_optionPatientConditionPageOnEditRecord = NULL;
	m_optionPatientPicPageOnEditRecord = NULL;

	m_labWaitMeetingStart = NULL;

	m_labPageNumOnAppoint = NULL;
	m_labCurrentPageNumOnAppoint = NULL;
	m_labPageNumOnRecord = NULL;
	m_labCurrentPageNumOnRecord = NULL;

}
void CDuiFrameWnd::InitSimpleData() {
	m_dataChooseOnSearchAppoint->SetText(_T(""));
	m_dataChooseOnSearchRecord->SetText(_T(""));

	hFile = CreateFileW(L"..\\hdlog.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA("create file error!\n");
		CloseHandle(hFile);
	}
	else {
		SetFilePointer(hFile, NULL, NULL, FILE_END);

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);

		CStringA logmsg = "初始化程序.\r\n";
		HDWriteLog(logmsg);
	}
	if (SDKAuth() == true) {
		OutputDebugStringW(L"sdk auth success.\n");
		CStringA logmsg = "会议身份验证成功。\r\n";
		HDWriteLog(logmsg);
	}
	else {
		OutputDebugStringW(L"sdk auth failed.\n");
		CStringA logmsg = "会议身份验证失败。\r\n";
		HDWriteLog(logmsg);
		rtmessage = L"会议身份验证失败！";
		::MessageBox(*this, rtmessage, _T("提示"), 0);
	}
	InitVoice();
};


LRESULT CDuiFrameWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
			case VK_RETURN:
			case VK_ESCAPE://拦截ESC退出界面
				bHandled = true;
				return 0;
			default:
				break;
		}
	}
	return FALSE;
}


LRESULT CDuiFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if (uMsg == WM_CLOSE)
	{
		OnClose(uMsg, wParam, lParam, bHandled);
		return lRes;
	}
	else  if(uMsg == WM_TIMER){
      OnTimer(uMsg, wParam, lParam, bHandled); 
	  return lRes;
	}
	else if (uMsg == WM_DESTROY)
	{
		OnDestroy(uMsg, wParam, lParam, bHandled);
		return lRes;
	}
/*
	if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	}
	*/
	return __super::HandleMessage(uMsg, wParam, lParam);

}

void CDuiFrameWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	char aa[30];
	sprintf_s(aa, "---on timer id = %d", wParam);
	OutputDebugStringA(aa);
	if (timer1used == 1) {
		if (meetingfailed == 1) {
			KillTimer(*this, 1);
			timer1used = 0;
		}
		if (m_pAuthServiceMgr->m_bLogined == true) {
			if (m_pMeetingServiceMgr->inmeetingnow == 0) {
				SDKStartMeeting();
			}
			else {
				KillTimer(*this, 1);
				timer1used = 0;
			}
		}
	}

	if (timer2used == 1) {
		if (m_pAuthServiceMgr->m_bLogined == false) {
			KillTimer(*this, 2);
			timer2used = 0;
			if (SDKLogin() == true) {
				OutputDebugStringW(L"sdk login success.\n");
				CStringA logmsg = "会议登录成功。\r\n";
				HDWriteLog(logmsg);
				SetTimer(*this, 1, 2000, NULL);
				timer1used = 1;
			}
			else {
				OutputDebugStringW(L"sdk login failed.\n");
				m_labWaitMeetingStart->SetVisible(false);
				CStringA logmsg = "会议登录失败。\r\n";
				HDWriteLog(logmsg);
				rtmessage = L"视频会议登录失败！";
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
	}

	//bHandled = TRUE;
	//return 0;
}

LRESULT CDuiFrameWnd::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	OutputDebugStringA("close frame \n ");
	ResetAllControls();
	bHandled = FALSE;
	m_pAuthServiceMgr->UnInit();
	m_pMeetingServiceMgr->UnInit();
	UninitVoice();
	KillTimer(*this, 1);
	KillTimer(*this, 2);
	timer1used = 0;
	timer2used = 0;
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	system("del ..\\hdtemp\\*.* /a /s /f /q");
	PostQuitMessage(0);
	return 0;
}

LRESULT CDuiFrameWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	OutputDebugStringA("destroy frame \n ");
	ResetAllControls();
	bHandled = FALSE;
	m_pAuthServiceMgr->UnInit();
	m_pMeetingServiceMgr->UnInit();
	UninitVoice();
	KillTimer(*this, 1);
	KillTimer(*this, 2);
	timer1used = 0;
	timer2used = 0;
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	system("del ..\\hdtemp\\*.* /a /s /f /q");
	PostQuitMessage(0);
	return 0;
}



CControlUI* CDuiFrameWnd::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (_tcsicmp(pstrClassName, _T("Appointment")) == 0)
	{
		strXML = _T("Appointment.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("CurrentConsultation")) == 0)
	{
		strXML = _T("CurrentConsultation.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("ConsultationRecord")) == 0)
	{
		strXML = _T("ConsultationRecord.xml");
	}
	else 	if (_tcsicmp(pstrClassName, _T("ListItemButton")) == 0) {
		return  new CListItemButtonUI();
	}
	
	if (!strXML.IsEmpty())
	{
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}
	else {
		return NULL;
	}
}


void CDuiFrameWnd::OnClick(TNotifyUI &msg) {

	if (msg.pSender == m_btnLoginUI)
	{	
		CStringA logmsg = "登录。\r\n";
		HDWriteLog(logmsg);
		if (HttpLogin() > 0) {
			m_containerLoginUI->SetVisible(false);
			m_containerResetPwdUI->SetVisible(false);
			m_containerMainpageUI->SetVisible(true);
			m_labTitleExpertImageUI->SetVisible(true);
			m_containerTitleUI->SetBkImage(L"bkimage/top-bg.png");
			HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
		}
	}
	else if (msg.pSender->GetName() == _T("minbtn"))
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		return;
	}
	else if (msg.pSender == m_btnCloseEXEUI) {

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);


		CStringA logmsg = "退出程序。\r\n";
		HDWriteLog(logmsg);
		Close();
	}
	else if (msg.pSender == m_btnCloseUI) {
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[30];
		sprintf_s(aa, "%4d-%02d-%02d %02d:%02d:%02d   ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
		HDWriteLog(aa);

		CStringA logmsg = "关闭程序。\r\n";
		HDWriteLog(logmsg);
		Close();
	}
	else if (msg.pSender == m_btnToResetPwdUI) {
		CStringA logmsg = "进入修改密码页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(false);
		m_containerMainpageUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(true);
	}
	else if (msg.pSender == m_btnGetResetVerificaionUI) {
		CStringA logmsg = "获取验证码。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnResetPwd"))->GetText();;
		if (HttpSendCode(mobile) > 0) {

		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}
	else if (msg.pSender == m_btnResetPwdUI) {
		CStringA logmsg = "重置密码。\r\n";
		HDWriteLog(logmsg);
		CStringW mobile = m_PaintManager.FindControl(_T("EditPhoneOnResetPwd"))->GetText();
		CStringW code = m_PaintManager.FindControl(_T("EditVerificationOnResetPwd"))->GetText();
		CStringW passwd = m_PaintManager.FindControl(_T("EditNewPwdOnResetPwd"))->GetText();
		CStringW repasswd = m_PaintManager.FindControl(_T("EditRepetPwdOnResetPwd"))->GetText();

		CStringA poststr = "mobile=";
		poststr += WStringToUTF8(mobile).c_str();
		poststr += "&code=";
		poststr += WStringToUTF8(code).c_str();
		poststr += "&password=";
		poststr += WStringToUTF8(passwd).c_str();
		poststr += "&password_confirm=";
		poststr += WStringToUTF8(repasswd).c_str();

		if (HttpResetPassword(poststr) > 0) {
			m_containerLoginUI->SetVisible(true);
			m_containerMainpageUI->SetVisible(false);
			m_containerResetPwdUI->SetVisible(false);
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonBackToLoginOnResetPwd")) {
		CStringA logmsg = "返回登录页面。\r\n";
		HDWriteLog(logmsg);
		m_containerLoginUI->SetVisible(true);
		m_containerMainpageUI->SetVisible(false);
		m_containerResetPwdUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSearchAppointment")) {
		CStringA logmsg = "搜索预约。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnSearchAppoint->GetText();
		CStringW clinicname = m_PaintManager.FindControl(_T("EditChooseClinicOnSearchAppoint"))->GetText();
		CStringW curl = APPOINTMENT_LIST_URL;
		appointsearchtext = L"";

		if (clinicname != _T("")) {
			curl += L"&clinicName=";
			curl += clinicname;
			appointsearchtext += L"&clinicName=";
			appointsearchtext += clinicname;
		}
		if (choosedate != _T("")) {
			curl += L"&date=";
			curl += choosedate;
			appointsearchtext += L"&date=";
			appointsearchtext += choosedate;
		}
		OutputDebugStringW(curl);
		OutputDebugStringW(L"\r\n");
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, 1);
	}
	else if (msg.pSender == m_btnShowPatientPic1UI) {
		CStringA logmsg = "查看患者大图1。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic2UI) {
		CStringA logmsg = "查看患者大图2。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic3UI) {
		CStringA logmsg = "查看患者大图3。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnReturnToAppointmentListUI) {
		CStringA logmsg = "返回预约列表。\r\n";
		HDWriteLog(logmsg);
		m_btnShowPatientPic1UI->SetVisible(true);
		m_btnShowPatientPic2UI->SetVisible(true);
		m_btnShowPatientPic3UI->SetVisible(true);
		m_containerAppointmentListUI->SetVisible(true);
		m_containerAppointmentDetailUI->SetVisible(false);
	}


	//record part
	else if (msg.pSender->GetName() == _T("ButtonSearchRecord")) {
		CStringA logmsg = "查找诊断记录。\r\n";
		HDWriteLog(logmsg);
		CStringW choosedate = m_dataChooseOnSearchRecord->GetText();
		CStringW patientname = m_PaintManager.FindControl(_T("EditPatientNameOnSearchRecord"))->GetText();
		CStringW clinicname = m_PaintManager.FindControl(_T("EditClinicNameOnSearchRecord"))->GetText();
		CStringW curl = RECORD_LIST_URL;
		recordsearchtext = L"";

		if (clinicname != _T("")) {
			curl += L"&clinicName=";
			curl += clinicname;
			recordsearchtext += L"&clinicname=";
			recordsearchtext += clinicname;
		}
		if (choosedate != _T("")) {
			curl += L"&date=";
			curl += choosedate;
			recordsearchtext += L"&date=";
			recordsearchtext += choosedate;
		}
		if (patientname != _T("")) {
			curl += L"&patient_name=";
			curl += patientname;
			recordsearchtext += L"&date=";
			recordsearchtext += choosedate;
		}
		OutputDebugStringW(curl);
		OutputDebugStringW(L"\r\n");
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, 1);
	}


	else if (msg.pSender == m_btnPlayAudio) {
		CStringA logmsg = "播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 0) {
			CStringW temp = m_btnPlayAudio->GetUserData();
			if (temp == L"") {
				::MessageBox(*this, _T("音频尚未生成。"), _T("提示"), 0);
			}
			else {
				temp += L"&token=";
				temp += token;
				CStringA path1 = temp;
				OutputDebugStringA(path1);
				OutputDebugStringA("\r\n");
				if (m_pAVPlayer->Play((LPCSTR)path1)) {		
					m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play-no.png");
					m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
					m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
					m_btnPlayAudio->SetEnabled(false);
					m_btnPauseAudio->SetVisible(true);
					m_btnStopAudio->SetVisible(true);
					inplaying = 1;
				}
				else {
					::MessageBox(*this, _T("播放失败。"), _T("提示"), 0);
				}
			}
		}
		else if (inplaying == 1) {
			m_pAVPlayer->Play();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play-no.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetEnabled(false);
			m_btnPauseAudio->SetEnabled(true);
		}
	}

	else if (msg.pSender == m_btnPauseAudio) {
		CStringA logmsg = "暂停播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Pause();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended-no.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetEnabled(false);
		}
	}

	else if (msg.pSender == m_btnStopAudio) {
		CStringA logmsg = "停止播放音频。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Stop();
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetVisible(true);
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetVisible(false);
			m_btnPauseAudio->SetEnabled(true);
			m_btnStopAudio->SetVisible(false);
			inplaying = 0;
		}
	}


	else if (msg.pSender->GetName() == _T("ButtonReturnToRecordList")) {
		CStringA logmsg = "返回诊断列表。\r\n";
		HDWriteLog(logmsg);
		if (inplaying == 1) {
			m_pAVPlayer->Stop();
			inplaying = 0;
			m_btnPlayAudio->SetBkImage(L"Button/cont-audio-play.png");
			m_btnPauseAudio->SetBkImage(L"Button/cont-audio-suspended.png");
			m_btnStopAudio->SetBkImage(L"Button/cont-audio-stop.png");
			m_btnPlayAudio->SetVisible(true);
			m_btnPlayAudio->SetEnabled(true);
			m_btnPauseAudio->SetVisible(false);
			m_btnPauseAudio->SetEnabled(true);
			m_btnStopAudio->SetVisible(false);
		}
		m_btnShowPatientPic1UIOnRDDetail->SetVisible(true);
		m_btnShowPatientPic2UIOnRDDetail->SetVisible(true);
		m_btnShowPatientPic3UIOnRDDetail->SetVisible(true);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonSubmitEditRecord")) {
		CStringA logmsg = "确认修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		CStringW advise = m_PaintManager.FindControl(_T("EditExpertAdviseOnRecordEdit"))->GetText();
		CStringA poststr = "source=expert";
		poststr += "&appointment_no=";
		poststr += editrecordappointmentno;
		if (advise != _T("")) {
			poststr += "&expert_advise=";
			poststr += WStringToUTF8(advise).c_str();
			if (HttpUpdateRecord(poststr) > 0) {
				m_containerRecordListUI->SetVisible(true);
				m_containerRecordDetailUI->SetVisible(false);
				m_containerEditRecordUI->SetVisible(false);
			}
			else {
				::MessageBox(*this, rtmessage, _T("提示"), 0);
			}
		}
		else {
			::MessageBox(*this, _T("缺少诊断信息！"), _T("提示"), 0);
		}

	}
	else if (msg.pSender->GetName() == _T("ButtonCancelSubmitEditRecord")) {
		CStringA logmsg = "取消修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender->GetName() == _T("ButtonReturnToRecordListOnEditRD")) {
		CStringA logmsg = "返回诊断列表。\r\n";
		HDWriteLog(logmsg);
		m_containerRecordListUI->SetVisible(true);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(false);
	}

	else if (msg.pSender == m_btnShowPatientPic1UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图1。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic2UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图2。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}
	else if (msg.pSender == m_btnShowPatientPic1UIOnRDDetail) {
		CStringA logmsg = "诊断记录详情页面查看患者大图3。\r\n";
		HDWriteLog(logmsg);
		POINT pt = { 0, 0 };
		CStringW temp = msg.pSender->GetUserData();
		if (temp != L"") {
			BSTR theurl = temp.AllocSysString();
			m_pShowPatientBigPicWnd = new CDiagShowPatientBigPicWnd();
			m_pShowPatientBigPicWnd->Init(*this, pt);
			m_pShowPatientBigPicWnd->InitData(theurl);
			m_pShowPatientBigPicWnd->ShowWindow();
		}
	}


	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfApppont")) {
		CStringW curl = APPOINTMENT_LIST_URL;
		curl += appointsearchtext;
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfApppont")) {
		CStringW curl = APPOINTMENT_LIST_URL;
		curl += appointsearchtext;
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(curl, appointlisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfApppont")) {
		if (appointlistcurrentpagenum > 1) {
			CStringW curl = APPOINTMENT_LIST_URL;
			curl += appointsearchtext;
			m_listAppointmentUI->RemoveAll();
			int pagenum = appointlistcurrentpagenum - 1;
			HttpGetAppointmentList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfApppont")) {
		if (appointlistcurrentpagenum < appointlisttotalpagenum) {
			CStringW curl = APPOINTMENT_LIST_URL;
			curl += appointsearchtext;
			m_listAppointmentUI->RemoveAll();
			int pagenum = appointlistcurrentpagenum + 1;
			HttpGetAppointmentList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonFirstPageOfRecord")) {
		CStringW curl = RECORD_LIST_URL;
		curl += recordsearchtext;
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, 1);
	}

	else if (msg.pSender->GetName() == _T("ButtonEndPageOfRecord")) {
		CStringW curl = RECORD_LIST_URL;
		curl += recordsearchtext;
		m_listRecordUI->RemoveAll();
		HttpGetRecordList(curl, recordlisttotalpagenum);
	}

	else if (msg.pSender->GetName() == _T("ButtonPreviousPageOfRecord")) {
		if (recordlistcurrentpagenum > 1) {
			CStringW curl = RECORD_LIST_URL;
			curl += recordsearchtext;
			int pagenum = recordlistcurrentpagenum - 1;
			m_listRecordUI->RemoveAll();
			HttpGetRecordList(curl, pagenum);
		}
		else {
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonNextPageOfRecord")) {
		if (recordlistcurrentpagenum < recordlisttotalpagenum) {
			CStringW curl = RECORD_LIST_URL;
			curl += recordsearchtext;
			int pagenum = recordlistcurrentpagenum + 1;
			m_listRecordUI->RemoveAll();
			HttpGetRecordList(curl, pagenum);
		}
		else {
		}
	}

}

void CDuiFrameWnd::OnItemButtonClick(TNotifyUI &msg) {
	if (msg.pSender->GetName() == _T("ButtonCheckAppointDetail"))
	{
		CStringA logmsg = "查看预约详情。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnShowAM->Selected(true);
		m_tabPatientInfoOnDetailControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowAM->SetBkColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		HttpGetAppointmentDetail(appointmentnumber);
		m_containerAppointmentListUI->SetVisible(false);
		m_containerAppointmentDetailUI->SetVisible(true);
	}


	else if (msg.pSender->GetName() == _T("ButtonCheckRecordDetail"))
	{
		CStringA logmsg = "查看诊断记录详情。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnShowRecord->Selected(true);
		m_tabPatientInfoOnRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowRecord->SetTextColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		HttpGetRecordDetail(appointmentnumber);
		m_containerRecordListUI->SetVisible(false);
		m_containerRecordDetailUI->SetVisible(true);
		m_containerEditRecordUI->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("ButtonEditRecord"))
	{
		CStringA logmsg = "修改诊断记录。\r\n";
		HDWriteLog(logmsg);
		m_optionPatientConditionPageOnEditRecord->Selected(true);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0x00000000);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		m_containerRecordListUI->SetVisible(false);
		m_containerRecordDetailUI->SetVisible(false);
		m_containerEditRecordUI->SetVisible(true);
		HttpGetRecordUpdate(appointmentnumber);
	}

	else if (msg.pSender->GetName() == _T("ButtonSendStartMsg")) {
		CStringA poststr = L"appointment_no=";
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		poststr += appointmentnumber;
		poststr += "&op=check";

		if (HttpCheckSendStartMsg(poststr) > 0) {
			int res = ::MessageBox(*this, _T("是否通知诊所提前开始会诊？"), _T("提示"), MB_YESNO);
			if (res == IDNO) {
				OutputDebugStringA("not send start message\n");
			}
			else if (res == IDYES)
			{
				CStringA poststr1 = L"appointment_no=";
				poststr1 += appointmentnumber;
				poststr1 += "&op=sms";
				HttpSendStartMsg(poststr1);
			}
		}
		else {
			::MessageBox(*this, rtmessage, _T("提示"), 0);
		}
	}

	else if (msg.pSender->GetName() == _T("ButtonStartZoom"))
	{
		CStringA logmsg = "开始视频诊疗。\r\n";
		HDWriteLog(logmsg);
		meetingfailed = 0;		
	
		CStringA poststr = L"appointment_no=";
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		poststr += appointmentnumber;

		inmeetingappointmentno = appointmentnumber;

		if (HttpGetAppointmentStatus(appointmentnumber) == 1) {			
			if (m_pMeetingServiceMgr->inmeetingnow == 1) {
				::MessageBox(*this, L"正在会议中", L"提示", 0);	
			}
			else {
				m_pMeetingServiceMgr->UnInit();
				if (HttpCreateZhumuMeeting(poststr) > 0) {
					SetTimer(*this, 1, 2000, NULL);
					timer1used = 1;
				}
				else {
					if (rtmessage != L"") {
						::MessageBox(*this, rtmessage, L"提示", 0);
					}						
				}
			}
		}
		else {
			::MessageBox(*this, L"诊疗已完成，需要重新开始诊断请联系管理员。", L"提示", 0);
		}
	}
	else if (msg.pSender->GetName() == _T("ButtonToRecordAdviseOnCAMLine"))
	{
		CStringA logmsg = "打开记录诊断新窗口。\r\n";
		HDWriteLog(logmsg);
		LPCTSTR appointmentnumber = msg.pSender->GetParent()->GetParent()->GetUserData();
		POINT pt = { 0, 0 };
		m_pToRecordConsultationWnd = new CDiagToRecordConsultationWnd();
		m_pToRecordConsultationWnd->Init(*this, pt);
		m_pToRecordConsultationWnd->InitRecordData(appointmentnumber, token);
		//m_pToRecordConsultationWnd->ShowWindow();
		m_pToRecordConsultationWnd->ShowModal();
	}
}


void CDuiFrameWnd::OnSelectChanged(TNotifyUI &msg) {
	CDuiString name = msg.pSender->GetName();
	if (name == _T("CheckAppointment")) {
		CStringA logmsg = "进入查看预约tab。\r\n";
		HDWriteLog(logmsg);
		m_btnCloseEXEUI->SetVisible(false);
		m_tabMainControlUI->SelectItem(0);
		m_dataChooseOnSearchAppoint->SetText(_T(""));
		appointsearchtext = L"";
		m_PaintManager.FindControl(_T("EditChooseClinicOnSearchAppoint"))->SetText(_T(""));
		m_labWaitMeetingStart->SetVisible(false);
		m_tabMainControlUI->SelectItem(0);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_selected.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");
		m_listAppointmentUI->RemoveAll();
		HttpGetAppointmentList(APPOINTMENT_LIST_URL, 1);
	}		
	else if (name == _T("CurrentConsultation")) {
		CStringA logmsg = "进入当日诊疗tab。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(1);
		m_btnCloseEXEUI->SetVisible(true);
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_selected.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_normal.png");

		m_labWaitMeetingStart->SetVisible(false);

		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char aa[20];
		sprintf_s(aa, "%4d-%02d-%02d", sys.wYear, sys.wMonth, sys.wDay);
		CStringW bb = aa;
		m_PaintManager.FindControl(_T("LabelDateUI"))->SetText(bb);
		CStringW curl = CURRENT_APPOINTMENT_LIST_URL;
		curl += L"&date=";
		curl += bb;
		//curl += L"2017-07-18";
		m_listCAppointmentUI->RemoveAll();
		HttpGetCurrentAppointmentList(curl);
	}
	else if (name == _T("ConsultationRecord")) {
		CStringA logmsg = "进入诊断记录页面。\r\n";
		HDWriteLog(logmsg);
		m_tabMainControlUI->SelectItem(2);
		m_btnCloseEXEUI->SetVisible(false);
		m_labWaitMeetingStart->SetVisible(false);
		m_dataChooseOnSearchRecord->SetText(_T(""));
		recordsearchtext = L"";
		m_optionCheckAppointment->SetBkImage(L"Option/checkappointment_normal.png");
		m_optionConsultationRecord->SetBkImage(L"Option/record_selected.png");
		m_optionCurrentConsultation->SetBkImage(L"Option/currentappointment_normal.png");

		m_dataChooseOnSearchRecord->SetText(_T(""));

		m_listRecordUI->RemoveAll();
		HttpGetRecordList(RECORD_LIST_URL, 1);
	}
	else if (name == _T("OptionPatientConditionPageOnDetail")) {
		CStringA logmsg = "预约详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnDetailControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowAM->SetBkColor(0x00000000);
	}
		
	else if (name == _T("OptionPatientPicPageOnDetail")) {
		CStringA logmsg = "预约详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnDetailControlUI->SelectItem(1);
		m_optionPatientConditionPageOnShowAM->SetBkColor(0x00000000);
		m_optionPatientPicPageOnShowAM->SetBkColor(0xffffffff);
	}

	else if (name == _T("OptionPatientConditionPageOnRecord")) {
		CStringA logmsg = "诊断记录详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnShowRecord->SetBkColor(0x00000000);
	}

	else if (name == _T("OptionPatientPicPageOnRecord")) {
		CStringA logmsg = "诊断记录详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnRecordControlUI->SelectItem(1);
		m_optionPatientConditionPageOnShowRecord->SetBkColor(0x00000000);
		m_optionPatientPicPageOnShowRecord->SetBkColor(0xffffffff);
	}


	else if (name == _T("OptionPatientConditionPageOnEditRecord")) {
		CStringA logmsg = "修改诊断记录详情页面进入患者主述tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(0);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0xffffffff);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0x00000000);
	}

	else if (name == _T("OptionPatientPicPageOnEditRecord")) {
		CStringA logmsg = "修改诊断记录详情页面进入患者图像tab。\r\n";
		HDWriteLog(logmsg);
		m_tabPatientInfoOnEditRecordControlUI->SelectItem(1);
		m_optionPatientConditionPageOnEditRecord->SetBkColor(0x00000000);
		m_optionPatientPicPageOnEditRecord->SetBkColor(0xffffffff);
	}

}


