#pragma once
#include "afxdialogex.h"
#include "FuncLib.h"
#include "CRegiPromoDlg.h"
#include "PromotionRegister.h"


// CSearchPromoDlg 대화 상자

class CSearchPromoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchPromoDlg)

public:
	CSearchPromoDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSearchPromoDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SearchPromo };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSearchpromo();
private:
	CRegiPromoDlg m_RegiPromoDlg;

	CString m_PromoID;

	long m_lTotalRecNum;
	RegiPromoDlgRec* m_pRPDR;

	CListCtrl m_ListCtrlSearchedPromos;

	void CSearchPromoDlg::InsertpRPDR();
public:
	afx_msg void OnBnClickedButtonRegipromo();
	afx_msg void OnBnClickedButtonDeletepromo();
};
