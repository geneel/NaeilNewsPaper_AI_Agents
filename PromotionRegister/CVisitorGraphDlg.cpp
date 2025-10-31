// CVisitorGraphDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CVisitorGraphDlg.h"


// CVisitorGraphDlg 대화 상자

IMPLEMENT_DYNAMIC(CVisitorGraphDlg, CDialogEx)

CVisitorGraphDlg::CVisitorGraphDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_VisitorGraph, pParent)
	, m_VisitorCID(_T(""))
	, m_AnalRegDate(_T(""))
	, m_TotalEventSize(_T(""))
{
	memset(caVisitorCID, 0, sizeof(caVisitorCID)); memset(caAnalRegDate, 0, sizeof(caAnalRegDate));

	iTotalClassNum = iTotalWriterNum = iTotalEventSize = 0;
	memset(dNormalClass, 0, sizeof(dNormalClass)); memset(dNormalWriter, 0, sizeof(dNormalWriter));
	memset(dPeriod, 0, sizeof(dPeriod)); memset(dAccel, 0, sizeof(dAccel));
	memset(dArticleEmb, 0, sizeof(dArticleEmb)); memset(dQueryEmb, 0, sizeof(dQueryEmb));
}

CVisitorGraphDlg::~CVisitorGraphDlg()
{
}

void CVisitorGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VisitorCID, m_VisitorCID);
	DDX_Text(pDX, IDC_EDIT_AnalRegDate, m_AnalRegDate);
	DDX_Text(pDX, IDC_EDIT_TotalEventSize, m_TotalEventSize);
}


BEGIN_MESSAGE_MAP(CVisitorGraphDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_OnOK, &CVisitorGraphDlg::OnBnClickedButtonOnok)
END_MESSAGE_MAP()


// CVisitorGraphDlg 메시지 처리기
BOOL CVisitorGraphDlg::OnInitDialog()
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

void CVisitorGraphDlg::OnBnClickedButtonOnok()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnOK();
}
