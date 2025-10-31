#pragma once
#include "afxdialogex.h"
#include "FuncLib.h"
#include "CVisitorGraphDlg.h"
#include "PromotionRegister.h"


// CSearchVisitorDlg 대화 상자

class CSearchVisitorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchVisitorDlg)

public:
	CSearchVisitorDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSearchVisitorDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SearchVisitor };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
private:
	CListCtrl m_ListCtrlVisitorList;

	VisitorDocu* m_pVisitors;
	int m_VisitorNum;

	CString m_VisitorCID;
	CString m_VisitorName;
	CString m_VisitorPhone;
	CString m_VisitorEmail;
	CString m_VisitorID;
	CString m_VisitorRecommender;

	void CSearchVisitorDlg::InsertpVisitors(); 

	CVisitorGraphDlg m_VisitorGraphDlg;

public:
	afx_msg void OnBnClickedButtonSearchvisitor();
	afx_msg void OnBnClickedButtonVisitorgraph();
};
