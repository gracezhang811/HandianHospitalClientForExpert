#include "stdafx.h"
#include "DiagToRecordConsultation.h"

CStringA tokendiag("");
CStringA editrecordappointmentnodiag("");
CStringW rtmessage1("");

void CDiagToRecordConsultationWnd::HttpGetRecordUpdate(LPCTSTR appointmentnumber, CStringA token) {
	CStringW curl = USER_UPDATE_RECORD_URL;
	CStringW param = appointmentnumber;
	curl += appointmentnumber;
	OutputDebugStringW(curl);
	OutputDebugStringW(L"\r\n");
	Json::Value root = HDWinHttpGetWithToken(curl, token);

	if (root["code"] == 0) {
		CStringW temp;
		OutputDebugStringA("get record info update success \n ");
		temp = UTF8ToWString(root["data"]["clinic_name"].asCString());
		m_PaintManager.FindControl(_T("LabelClinicNameOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_starttime"].asCString());
		m_PaintManager.FindControl(_T("LabelStartTimeOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["real_endtime"].asCString());
		m_PaintManager.FindControl(_T("LabelEndTimeOnToRecord"))->SetText(temp);

		temp = UTF8ToWString(root["data"]["patient_description"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientDescOnToRecord"))->SetText(temp);

		if ((root["data"]["patient_img1_thumb"] != "") && (root["data"]["patient_img1_thumb"].isNull() == false)) {
			temp = UTF8ToWString(root["data"]["patient_img1_thumb"].asCString());
			CStringW path1 = L"..\\hdtemp\\";
			path1 += appointmentnumber;
			path1 += L"-pic1-";
			//int index = temp.ReverseFind('/');
			path1 += temp.Mid(60);
			if (PathFileExists(path1) == TRUE) {
				OutputDebugStringA("\r\n file exist.\n");
				m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(path1);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path1) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(path1);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic1OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
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
				m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(path2);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path2) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(path2);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic2OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
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
				m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(path3);
			}
			else {
				OutputDebugStringA("\r\n file not exist.\n");
				if (HDWinHttpDownloadFile(temp, path3) > 0) {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(path3);
					OutputDebugStringA("download success.\n");
				}
				else {
					m_PaintManager.FindControl(_T("ConteolShowPatientPic3OnToRecord"))->SetBkImage(L"bkimage/downloadimagefailed-1.png");
				}
			}
		}

		temp = UTF8ToWString(root["data"]["patient_name"].asCString());
		m_PaintManager.FindControl(_T("LabelPatientNameOnToRecord"))->SetText(temp);


		int age = root["data"]["patient_age"].asInt();
		if (age != 0) {
			char aa[20];
			sprintf_s(aa, "%d", age);
			CStringW bb = aa;
			m_PaintManager.FindControl(_T("LabelPatientAgeOnToRecord"))->SetText(bb);
		}

		int sex = root["data"]["patient_gender"].asInt();
		if (sex == 2) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnToRecord"))->SetText(_T("Ů"));
		}
		else if (sex == 1) {
			m_PaintManager.FindControl(_T("LabelPatientSexOnToRecord"))->SetText(_T("��"));
		}

		if (root["data"]["patient_mobile"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_mobile"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientPhoneOnToRecord"))->SetText(temp);
		}

		if (root["data"]["patient_idcard"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["patient_idcard"].asCString());
			m_PaintManager.FindControl(_T("LabelPatientIDOnToRecord"))->SetText(temp);

		}

		if (root["data"]["expert_diagnosis"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_diagnosis"].asCString());
			m_PaintManager.FindControl(_T("LabelDiagnosisDescOnToRecord"))->SetText(temp);
		}

		if (root["data"]["expert_advise"].isNull() == false) {
			temp = UTF8ToWString(root["data"]["expert_advise"].asCString());
			m_PaintManager.FindControl(_T("EditExpertAdviseOnToRecord"))->SetText(temp);
		}
	}
	else {
		OutputDebugStringA("get record info update failed \n ");
	}
}


int CDiagToRecordConsultationWnd::HttpUpdateRecord(CStringA poststr, CStringA token) {
	OutputDebugStringA(poststr);

	Json::Value root = HDWinHttpPostWithToken(USER_UPDATE_RECORD_SUBMIT_URL, poststr, token);

	if (root["code"] == 0) {
		OutputDebugStringA("update record success \n ");
		return 1;
	}
	else {
		OutputDebugStringA("update record failed \n ");
		rtmessage1 = UTF8ToWString(root["message"].asCString());
		return -1;
	}
}

LRESULT  CDiagToRecordConsultationWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Close();
	bHandled = FALSE;
	return 0;
}

void CDiagToRecordConsultationWnd::InitAllControls() {
	m_tabPatientInfoOnToRecordControlUI = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("switchPatientInfoTypeOnToRecord")));
	m_optionChooseMaleOnToRecord = (COptionUI*)m_PaintManager.FindControl(_T("CheckPatientmaleOnToRecord"));
	m_optionChooseFemaleOnToRecord = (COptionUI*)m_PaintManager.FindControl(_T("CheckPatientFemaleOnToRecord"));
	m_optionPatientConditionPageOnToRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientConditionPageOnToRecord")));
	m_optionPatientPicPageOnToRecord = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("OptionPatientPicPageOnToRecord")));
	m_richeditExpertAdvideOnToRecord = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("EditExpertAdviseOnToRecord")));
	m_btnStartVoiceOnToRecord = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonStartVoiceOnToRecord")));
	m_btnEndVoiceOnToRecord = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("ButtonEndVoiceOnToRecord")));

}

void CDiagToRecordConsultationWnd::ResetAllControls() {
	m_tabPatientInfoOnToRecordControlUI = NULL;
	m_optionChooseMaleOnToRecord = NULL;
	m_optionChooseFemaleOnToRecord = NULL;
	m_optionPatientConditionPageOnToRecord = NULL;
	m_optionPatientPicPageOnToRecord = NULL;
	m_richeditExpertAdvideOnToRecord = NULL;
	m_btnStartVoiceOnToRecord = NULL;
	m_btnEndVoiceOnToRecord = NULL;
}


LRESULT CDiagToRecordConsultationWnd::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ResetAllControls();	
	Close();
	bHandled = FALSE;
	return 0;
}

LRESULT CDiagToRecordConsultationWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	ResetAllControls();
	Close();
	bHandled = FALSE;
	return 0;
}

void CDiagToRecordConsultationWnd::InitRecordData(LPCTSTR appointmentnumber, CStringA token)
{
	tokendiag = token;
	editrecordappointmentnodiag = appointmentnumber;
	HttpGetRecordUpdate(appointmentnumber, token);
}

void CDiagToRecordConsultationWnd::Init(HWND hWndParent, POINT ptPos)
{
	Create(NULL, _T("��ϼ�¼"), UI_WNDSTYLE_DIALOG, WS_EX_WINDOWEDGE);
	//Create(hWndParent, _T("RecordConsulltationFrame"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	::ClientToScreen(hWndParent, &ptPos);
	::SetWindowPos(*this, NULL, ptPos.x, ptPos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	m_pVoiceResult = (char*)malloc(4096);//BUFFER_SIZE
	memset(m_pVoiceResult, 0, 4096);
	InitAllControls();
	
}

void	CDiagToRecordConsultationWnd::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("click"))
	{
		if (msg.pSender->GetName() == _T("ButtonSubmitRecordOnDiag"))
		{
			CStringW advise = m_richeditExpertAdvideOnToRecord->GetText();
			CStringA poststr = "source=expert";
			poststr += "&appointment_no=";
			poststr += editrecordappointmentnodiag;
			if (advise != _T("")) {
				poststr += "&expert_advise=";
				poststr += WStringToUTF8(advise).c_str();
				if (HttpUpdateRecord(poststr, tokendiag)) {
					Close();
				}
				else {
					::MessageBox(*this, rtmessage1, _T("��ʾ"), 0);
				}				
			}
			else {
				::MessageBox(*this, _T("ȱ�������Ϣ��"), _T("��ʾ"), 0);
			}		
			return;
		}
		if (msg.pSender->GetName() == _T("ButtonCancelSubmitRecordOnDiag"))
		{
			Close();
			return;
		}
		if (msg.pSender->GetName() == _T("ButtonCloseRecordDiag"))
		{
			Close();
			return;
		}
		if (msg.pSender == m_btnStartVoiceOnToRecord)
		{
			RecognizeVoice(&m_pVoiceResult);
			m_btnStartVoiceOnToRecord->SetTextColor(0xFFe3e4e8);
			m_btnStartVoiceOnToRecord->SetEnabled(false);
			m_btnEndVoiceOnToRecord->SetTextColor(0xFF313131);
			m_btnEndVoiceOnToRecord->SetEnabled(true);
		}
		if (msg.pSender == m_btnEndVoiceOnToRecord)
		{
			CStringW rtmessage("����ʶ�����");
			::MessageBox(*this, rtmessage, _T("��ʾ"), 0);
			CStringW temp = m_richeditExpertAdvideOnToRecord->GetText();
			CStringW temp1 = (LPCTSTR)AnsiToUnicode(m_pVoiceResult);
			temp += temp1;
			m_richeditExpertAdvideOnToRecord->SetText(temp);
			OutputDebugStringW((LPCWSTR)AnsiToUnicode(m_pVoiceResult));
			memset(m_pVoiceResult, 0, 4096);
			m_btnEndVoiceOnToRecord->SetTextColor(0xFFe3e4e8);
			m_btnEndVoiceOnToRecord->SetEnabled(false);
			m_btnStartVoiceOnToRecord->SetTextColor(0xFF313131);
			m_btnStartVoiceOnToRecord->SetEnabled(true);
		}

	}
	if (msg.sType == _T("selectchanged"))
	{
		if (msg.pSender->GetName() == _T("OptionPatientConditionPageOnToRecord")) {
			m_tabPatientInfoOnToRecordControlUI->SelectItem(0);
			m_optionPatientConditionPageOnToRecord->SetBkColor(0xffffffff);
			m_optionPatientPicPageOnToRecord->SetBkColor(0x00000000);
		}
			
		else if (msg.pSender->GetName() == _T("OptionPatientPicPageOnToRecord")) {
			m_tabPatientInfoOnToRecordControlUI->SelectItem(1);
			m_optionPatientConditionPageOnToRecord->SetBkColor(0x00000000);
			m_optionPatientPicPageOnToRecord->SetBkColor(0xffffffff);
		}
			
	}
}



LRESULT CDiagToRecordConsultationWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE://����ESC�˳�����
			bHandled = true;
			return 0;
		default:
			break;
		}
	}
	return FALSE;
}

LRESULT CDiagToRecordConsultationWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL    bHandled = TRUE;

	switch (uMsg)
	{
	case WM_CLOSE:
		lRes = OnClose(uMsg, wParam, lParam, bHandled);
		break;

	case WM_DESTROY:
		lRes = OnDestroy(uMsg, wParam, lParam, bHandled);
		break;

	case WM_KILLFOCUS:
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		break;
	default:
		bHandled = FALSE;
	}
	/*
	if (bHandled || m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	}
	*/
	return __super::HandleMessage(uMsg, wParam, lParam);
}









