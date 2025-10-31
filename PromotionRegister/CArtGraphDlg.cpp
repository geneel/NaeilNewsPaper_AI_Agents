// CArtGraphDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CArtGraphDlg.h"


// CArtGraphDlg 대화 상자

IMPLEMENT_DYNAMIC(CArtGraphDlg, CDialogEx)

CArtGraphDlg::CArtGraphDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ArtGraph, pParent)
	, m_ArtID(_T(""))
	, m_ArtRegDate(_T(""))
{
	memset(caArtID, 0, sizeof(caArtID)); memset(caArtRegDate, 0, sizeof(caArtRegDate));

	iTotalClassNum = iTotalWriterNum = 0;
	memset(dNormalClass, 0, sizeof(dNormalClass)); memset(dNormalWriter, 0, sizeof(dNormalWriter));
	memset(dArticleEmb, 0, sizeof(dArticleEmb)); memset(dQueryEmb, 0, sizeof(dQueryEmb));
}

CArtGraphDlg::~CArtGraphDlg()
{
}

void CArtGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ArtID, m_ArtID);
	DDX_Text(pDX, IDC_EDIT_ArtRegDate, m_ArtRegDate);
}


BEGIN_MESSAGE_MAP(CArtGraphDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_OnOK, &CArtGraphDlg::OnBnClickedButtonOnok)
END_MESSAGE_MAP()


// CArtGraphDlg 메시지 처리기
BOOL CArtGraphDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();   // 또는 CDialog::OnInitDialog()

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

void CArtGraphDlg::OnBnClickedButtonOnok()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnOK();
}
