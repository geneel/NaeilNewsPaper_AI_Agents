#pragma once
#include "afxdialogex.h"
#include "CPromoGraphDlg.h"


// CRegiPromoDlg 대화 상자

class CRegiPromoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRegiPromoDlg)

public:
	CRegiPromoDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CRegiPromoDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RegiPromo };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CPromoGraphDlg m_PromoGraphDlg;
public:
	CString m_PromoID;
	CString m_ArtIDs;
	CString m_SearchKeywords;
	CString m_VisitorPattern;

	afx_msg void OnBnClickedButtonPromograph();
};
