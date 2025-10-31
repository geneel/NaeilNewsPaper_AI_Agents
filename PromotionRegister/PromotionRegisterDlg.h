
// PromotionRegisterDlg.h: 헤더 파일
//

#pragma once

#include "CRegiPromoDlg.h"
#include "CSearchPromoDlg.h"
#include "CSearchArtDlg.h"
#include "CSearchVisitorDlg.h"


// CPromotionRegisterDlg 대화 상자
class CPromotionRegisterDlg : public CDialogEx
{
// 생성입니다.
public:
	CPromotionRegisterDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROMOTIONREGISTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	CRegiPromoDlg m_RegiPromoDlg;
	CSearchPromoDlg m_SearchPromoDlg;
	CSearchArtDlg m_SearchArtDlg;
	CSearchVisitorDlg m_SearchVisitorDlg;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonNewpromo();
	afx_msg void OnBnClickedButtonSearchpromo();
	afx_msg void OnBnClickedButtonSearchart();
	afx_msg void OnBnClickedButtonSearchvst();
};
