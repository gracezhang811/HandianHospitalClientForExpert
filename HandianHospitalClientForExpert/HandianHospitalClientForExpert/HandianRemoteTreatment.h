#pragma once

#include "Resource.h"
#include "duilib.h"
#include "zhumu_sdk.h"
#include "HDHttpFunction.h"
#include "def.h"
#include "json.h"
#include "DiagShowPatientBigPic.h"
#include "DiagToRecordConsultation.h"
#include "ListItemButtonUI.h"
#include "AVPlayer.h"
#include "vlc.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "myvoice.h"
#ifdef __cplusplus
}
#endif

class CAuthServiceMgr;
class CMeetingServiceMgr;


class CDuiFrameWnd : public WindowImplBase
{
public:
	CDuiFrameWnd();
	virtual ~CDuiFrameWnd();

public:
	virtual LPCTSTR		GetWindowClassName() const { return _T("DUIMainFrame"); };
	virtual CDuiString	GetSkinFile() { return _T("MainPage.xml"); };
	virtual CDuiString	GetSkinFolder() { return _T("../skin"); };

	virtual void		InitWindow();
	virtual LRESULT		HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
	void				InitSimpleData();
	LRESULT				OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT				OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void				OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	virtual CControlUI* CreateControl(LPCTSTR pstrClassName);


	DUI_DECLARE_MESSAGE_MAP()
	virtual void		OnClick(TNotifyUI& msg);
	virtual void		OnSelectChanged(TNotifyUI &msg);
	virtual void		OnItemButtonClick(TNotifyUI &msg);

private:
	void				InitAllControls();
	void				ResetAllControls();
	bool				SDKAuth();
	bool				SDKLogin();
	void				SDKStartMeeting();
	void				HDWriteLog(CStringA logmsg);

private:
	void				HttpGetAppointmentList(LPCTSTR curl, int pagenum);
	void				HttpGetCurrentAppointmentList(LPCTSTR curl);
	int					HttpLogin();
	int					HttpGetUserInfo();
	void				HttpGetAppointmentDetail(LPCTSTR appointmentnumber);
	int					HttpGetAppointmentStatus(LPCTSTR appointmentnumber);
	void				HttpGetRecordList(LPCTSTR curl, int pagenum);
	void				HttpGetRecordDetail(LPCTSTR appointmentnumber);
	void				HttpGetRecordUpdate(LPCTSTR appointmentnumber);
	int					HttpUpdateRecord(CStringA poststr);
	int					HttpSendCode(LPCTSTR mobile);
	int					HttpResetPassword(CStringA poststr);
	int					HttpCreateZhumuMeeting(CStringA poststr);
	int					HttpCheckSendStartMsg(CStringA poststr);
	void				HttpSendStartMsg(CStringA poststr);
	//void				HttpSendZhumuEndMesg(CStringA poststr);


private:
	CAuthServiceMgr*		m_pAuthServiceMgr;
	CMeetingServiceMgr*		m_pMeetingServiceMgr;
	CAVPlayer*				m_pAVPlayer;
	bool					m_bSDKInit;
	bool					m_bZhumuAuth;

	char*					m_pVoiceResultOnEditRecord;

private:
	CDiagShowPatientBigPicWnd*		m_pShowPatientBigPicWnd;
	CDiagToRecordConsultationWnd*	m_pToRecordConsultationWnd;

private:
	CContainerUI*			m_containerTitleUI;
	CContainerUI*			m_containerLoginUI;
	CContainerUI*			m_containerResetPwdUI;
	CContainerUI*			m_containerMainpageUI;

	CContainerUI*			m_containerAppointmentListUI;
	CContainerUI*			m_containerAppointmentDetailUI;

	CEditUI*				m_editMobileOnLoginUI;
	CEditUI*				m_editPWDOnLoginUI;

	CButtonUI*				m_btnCloseUI;
	CButtonUI*				m_btnCloseEXEUI;
	CButtonUI*				m_btnWndMinUI;

	CButtonUI*				m_btnLoginUI;
	CButtonUI*				m_btnToResetPwdUI;
	CButtonUI*				m_btnGetResetVerificaionUI;
	CButtonUI*				m_btnResetPwdUI;
	
	CLabelUI*				m_labTitleExpertNameUI;
	CLabelUI*				m_labTitleExpertImageUI;

	CTabLayoutUI*           m_tabMainControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnDetailControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnRecordControlUI;
	CTabLayoutUI*           m_tabPatientInfoOnEditRecordControlUI;

	CListUI*				m_listAppointmentUI;
	CListUI*				m_listCAppointmentUI;
	CListUI*				m_listRecordUI;

	CContainerUI*			m_containerRecordListUI;
	CContainerUI*			m_containerRecordDetailUI;
	CContainerUI*			m_containerEditRecordUI;

	CButtonUI*				m_btnShowPatientPic1UI;
	CButtonUI*				m_btnShowPatientPic2UI;
	CButtonUI*				m_btnShowPatientPic3UI;
	CButtonUI*				m_btnReturnToAppointmentListUI;

	CButtonUI*				m_btnShowPatientPic1UIOnRDDetail;
	CButtonUI*				m_btnShowPatientPic2UIOnRDDetail;
	CButtonUI*				m_btnShowPatientPic3UIOnRDDetail;

	CButtonUI*				m_btnPlayAudio;
	CButtonUI*				m_btnPauseAudio;
	CButtonUI*				m_btnStopAudio;

	CDateTimeUI*			m_dataChooseOnSearchAppoint;
	CDateTimeUI*			m_dataChooseOnSearchRecord;

	COptionUI*				m_optionCheckAppointment;
	COptionUI*				m_optionCurrentConsultation;
	COptionUI*				m_optionConsultationRecord;

	COptionUI*				m_optionPatientConditionPageOnShowAM;
	COptionUI*				m_optionPatientPicPageOnShowAM;

	COptionUI*				m_optionPatientConditionPageOnShowRecord;
	COptionUI*				m_optionPatientPicPageOnShowRecord;
	COptionUI*				m_optionPatientConditionPageOnEditRecord;
	COptionUI*				m_optionPatientPicPageOnEditRecord;

	CLabelUI*				m_labWaitMeetingStart;
	CLabelUI*				m_labPageNumOnAppoint;
	CLabelUI*				m_labCurrentPageNumOnAppoint;
	CLabelUI*				m_labPageNumOnRecord;
	CLabelUI*				m_labCurrentPageNumOnRecord;


};
