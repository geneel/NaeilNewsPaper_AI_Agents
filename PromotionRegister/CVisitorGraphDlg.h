#pragma once
#include "afxdialogex.h"
#include "FuncLib.h"
#include "PromotionRegister.h"
#include "CVisitorPeriodCtrl.h"
#include "CVisitorAccelCtrl.h"
#include "CArtClassCtrl.h"
#include "CArtWriterCtrl.h"
#include "CArtEmbeddingCtrl.h"
#include "CSearchKeywordsEmbeddingCtrl.h"


// CVisitorGraphDlg 대화 상자

class CVisitorGraphDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVisitorGraphDlg)

public:
	CVisitorGraphDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CVisitorGraphDlg();
	virtual BOOL CVisitorGraphDlg::OnInitDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_VisitorGraph };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CString m_VisitorCID;
	CString m_AnalRegDate;
	CString m_TotalEventSize;

	CVisitorPeriodCtrl m_VisitorPeriodCtrl;
	CVisitorAccelCtrl m_VisitorAccelCtrl;
	CArtClassCtrl m_ArtClassCtrl;
	CArtWriterCtrl m_ArtWriterCtrl;
	CArtEmbeddingCtrl m_ArtEmbeddingCtrl;
	CSearchKeywordsEmbeddingCtrl m_SearchKeywordsEmbeddingCtrl;

	char caVisitorCID[READ_BUF_SIZE], caAnalRegDate[READ_BUF_SIZE];
	int iTotalClassNum, iTotalWriterNum, iTotalEventSize;
	double dNormalClass[TIME_SLOT_SIZE / 2], dNormalWriter[TIME_SLOT_SIZE / 2];
	double dPeriod[TIME_SLOT_SIZE / 2], dAccel[TIME_SLOT_SIZE];
	double dArticleEmb[EMB_SIZE], dQueryEmb[EMB_SIZE];
	afx_msg void OnBnClickedButtonOnok();
};
