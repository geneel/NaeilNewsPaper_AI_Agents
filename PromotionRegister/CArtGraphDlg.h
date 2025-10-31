#pragma once
#include "afxdialogex.h"
#include "FuncLib.h"
#include "PromotionRegister.h"
#include "CArtClassCtrl.h"
#include "CArtWriterCtrl.h"
#include "CArtEmbeddingCtrl.h"
#include "CSearchKeywordsEmbeddingCtrl.h"


// CArtGraphDlg 대화 상자

class CArtGraphDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CArtGraphDlg)

public:
	CArtGraphDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CArtGraphDlg();
	virtual BOOL CArtGraphDlg::OnInitDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ArtGraph };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CString m_ArtID;
	CString m_ArtRegDate;
	CArtClassCtrl m_ArtClassCtrl;
	CArtWriterCtrl m_ArtWriterCtrl;
	CArtEmbeddingCtrl m_ArtEmbeddingCtrl;
	CSearchKeywordsEmbeddingCtrl m_SearchKeywordsEmbeddingCtrl;

	char caArtID[READ_BUF_SIZE], caArtRegDate[READ_BUF_SIZE];
	int iTotalClassNum, iTotalWriterNum;
	double dNormalClass[TIME_SLOT_SIZE / 2], dNormalWriter[TIME_SLOT_SIZE / 2];
	double dArticleEmb[EMB_SIZE], dQueryEmb[EMB_SIZE];
	afx_msg void OnBnClickedButtonOnok();
};
