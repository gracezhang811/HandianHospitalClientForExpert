// HandianHospitalClientForExpert.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "HandianHospitalClientForExpert.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	::CoInitialize(NULL);

	CDuiFrameWnd *duiFrame = new CDuiFrameWnd;
	duiFrame->Create(NULL, _T("��ӭ��¼����������"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	duiFrame->InitWindow();
	duiFrame->InitSimpleData();
	duiFrame->CenterWindow();
	duiFrame->ShowModal();

	delete duiFrame;
	::CoUninitialize();

	return 0;
}