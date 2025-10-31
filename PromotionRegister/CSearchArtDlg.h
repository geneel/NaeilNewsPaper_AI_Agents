#pragma once
#include "afxdialogex.h"
#include "FuncLib.h"
#include "CArtGraphDlg.h"


// CSearchArtDlg 대화 상자

class CSearchArtDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSearchArtDlg)

public:
	CSearchArtDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSearchArtDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SearchArt };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CString m_ArtURL;
	CArtGraphDlg m_ArtGraphDlg;
public:
	afx_msg void OnBnClickedButtonArtgraph();
};
