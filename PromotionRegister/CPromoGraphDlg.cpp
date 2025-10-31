// CPromoGraphDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CPromoGraphDlg.h"


// CPromoGraphDlg 대화 상자

IMPLEMENT_DYNAMIC(CPromoGraphDlg, CDialogEx)

CPromoGraphDlg::CPromoGraphDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_PromoGraph, pParent)
	, m_PromoID(_T(""))
	, m_PromoRegDate(_T(""))
	, m_PromoStartDate(_T(""))
	, m_PromoEndDate(_T(""))
	, m_ArtIDs(_T(""))
	, m_SearchKeywords(_T(""))
{
	memset(caPromoID, 0, sizeof(caPromoID)); memset(caPromoRegDate, 0, sizeof(caPromoRegDate)); memset(caPromoStartDate, 0, sizeof(caPromoStartDate));
	memset(caPromoEndDate, 0, sizeof(caPromoEndDate)); memset(caVisitorPattern, 0, sizeof(caVisitorPattern));

	iArtIDsNum = iSearchKeywordsNum = 0;
	memset(caClassAxis, 0, sizeof(caClassAxis)); memset(caWriterAxis, 0, sizeof(caWriterAxis)); memset(caVisitings, 0, sizeof(caVisitings));
	for (int i = 0; i < MIN_TOKENS; i++)
	{
		memset(caaArtIDs[i], 0, sizeof(caaArtIDs[i])); memset(caaSearchKeywords[i], 0, sizeof(caaSearchKeywords[i]));
	}

	iTotalClassNum = iTotalWriterNum = 0;
	memset(dNormalClass, 0, sizeof(dNormalClass)); memset(dNormalWriter, 0, sizeof(dNormalWriter));
	memset(dPeriod, 0, sizeof(dPeriod)); memset(dAccel, 0, sizeof(dAccel));
	memset(dArticleEmb, 0, sizeof(dArticleEmb)); memset(dQueryEmb, 0, sizeof(dQueryEmb));
}

CPromoGraphDlg::~CPromoGraphDlg()
{
}

void CPromoGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PromoID, m_PromoID);
	DDX_Text(pDX, IDC_EDIT_BannerRegDate, m_PromoRegDate);
	DDX_Text(pDX, IDC_EDIT_BannerStartDate, m_PromoStartDate);
	DDX_Text(pDX, IDC_EDIT_BannerEndDate, m_PromoEndDate);
	DDX_Text(pDX, IDC_EDIT_ArtID, m_ArtIDs);
	DDX_Text(pDX, IDC_EDIT_SearchKeywords, m_SearchKeywords);
}


BEGIN_MESSAGE_MAP(CPromoGraphDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SavePromo, &CPromoGraphDlg::OnBnClickedButtonSavepromo)
END_MESSAGE_MAP()


// CPromoGraphDlg 메시지 처리기
BOOL CPromoGraphDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();   // 또는 CDialog::OnInitDialog()

	m_VisitorPeriodCtrl.SubclassDlgItem(IDC_STATIC_VisitorPeriod, this);
	m_VisitorPeriodCtrl.SetDataPtr(dPeriod, TIME_SLOT_SIZE / 2);

	m_VisitorAccelCtrl.SubclassDlgItem(IDC_STATIC_VisitorAccel, this);
	m_VisitorAccelCtrl.SetDataPtr(dAccel, TIME_SLOT_SIZE);

	m_ArtClassCtrl.SubclassDlgItem(IDC_STATIC_ArtClass, this);
	m_ArtClassCtrl.SetDataPtr(dNormalClass, iTotalClassNum);

	m_ArtWriterCtrl.SubclassDlgItem(IDC_STATIC_ArtWriter, this);
	m_ArtWriterCtrl.SetDataPtr(dNormalWriter, iTotalWriterNum);

	m_ArtEmbeddingCtrl.SubclassDlgItem(IDC_STATIC_ArtEmbedding, this);
	m_ArtEmbeddingCtrl.SetDataPtr(dArticleEmb, EMB_SIZE);

	m_SearchKeywordsEmbeddingCtrl.SubclassDlgItem(IDC_STATIC_SearchKeywordsEmbedding, this);
	m_SearchKeywordsEmbeddingCtrl.SetDataPtr(dQueryEmb, EMB_SIZE);

	return TRUE;
}

void CPromoGraphDlg::OnBnClickedButtonSavepromo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	int iSuccess = 1;

	iSuccess = InsertDocuInPromotionsCollection(theApp.BigData_Promotions_collection, \
		theApp.m_User, "", true, "", \
		caPromoID, caPromoRegDate, caPromoStartDate, caPromoEndDate, \
		caaArtIDs, iArtIDsNum, caaSearchKeywords, iSearchKeywordsNum, \
		caClassAxis, caWriterAxis, caVisitings, \
		dPeriod, TIME_SLOT_SIZE / 2, dAccel, TIME_SLOT_SIZE, \
		dNormalClass, iTotalClassNum, dNormalWriter, iTotalWriterNum, \
		dArticleEmb, EMB_SIZE, dQueryEmb, EMB_SIZE);

	EndWaitCursor();

	if (iSuccess == 0)
	{
		AfxMessageBox(_T("이 프로모션이 서버에 저장되었습니다."));
	}
	else
	{
		AfxMessageBox(_T("이 프로모션의 서버 저장에 실패하였습니다."));
	}
}
