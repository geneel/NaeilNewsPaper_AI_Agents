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


// CPromoGraphDlg 대화 상자

class CPromoGraphDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPromoGraphDlg)

public:
	CPromoGraphDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CPromoGraphDlg();
	virtual BOOL CPromoGraphDlg::OnInitDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PromoGraph };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CString m_PromoID;
	CString m_PromoRegDate;
	CString m_PromoStartDate;
	CString m_PromoEndDate;
	CString m_ArtIDs;
	CString m_SearchKeywords;
	CVisitorPeriodCtrl m_VisitorPeriodCtrl;
	CVisitorAccelCtrl m_VisitorAccelCtrl;
	CArtClassCtrl m_ArtClassCtrl;
	CArtWriterCtrl m_ArtWriterCtrl;
	CArtEmbeddingCtrl m_ArtEmbeddingCtrl;
	CSearchKeywordsEmbeddingCtrl m_SearchKeywordsEmbeddingCtrl;

	//CString VisitorPattern;

	char caPromoID[READ_BUF_SIZE], caPromoRegDate[READ_BUF_SIZE], caPromoStartDate[READ_BUF_SIZE], caPromoEndDate[READ_BUF_SIZE];
	char caVisitorPattern[TEXT_SIZE*10];

	int iArtIDsNum, iSearchKeywordsNum;
	char caaArtIDs[MIN_TOKENS][MAX_LENGTH], caaSearchKeywords[MIN_TOKENS][MAX_LENGTH];
	char caClassAxis[TEXT_SIZE], caWriterAxis[TEXT_SIZE], caVisitings[TEXT_SIZE * 8];

	int iTotalClassNum, iTotalWriterNum;
	double dNormalClass[TIME_SLOT_SIZE / 2], dNormalWriter[TIME_SLOT_SIZE / 2];
	double dPeriod[TIME_SLOT_SIZE / 2], dAccel[TIME_SLOT_SIZE];
	double dArticleEmb[EMB_SIZE], dQueryEmb[EMB_SIZE];
	afx_msg void OnBnClickedButtonSavepromo();
};
