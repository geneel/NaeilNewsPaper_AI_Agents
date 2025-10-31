// CSearchPromoDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CSearchPromoDlg.h"


// CSearchPromoDlg 대화 상자

IMPLEMENT_DYNAMIC(CSearchPromoDlg, CDialogEx)

CSearchPromoDlg::CSearchPromoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SearchPromo, pParent)
	, m_PromoID(_T(""))
{
	m_lTotalRecNum = 0;
	m_pRPDR = NULL;
}

CSearchPromoDlg::~CSearchPromoDlg()
{
}

void CSearchPromoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PromoID, m_PromoID);
	DDX_Control(pDX, IDC_LIST_SearchedPromos, m_ListCtrlSearchedPromos);
}


BEGIN_MESSAGE_MAP(CSearchPromoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SearchPromo, &CSearchPromoDlg::OnBnClickedButtonSearchpromo)
	ON_BN_CLICKED(IDC_BUTTON_RegiPromo, &CSearchPromoDlg::OnBnClickedButtonRegipromo)
	ON_BN_CLICKED(IDC_BUTTON_DeletePromo, &CSearchPromoDlg::OnBnClickedButtonDeletepromo)
END_MESSAGE_MAP()


// CSearchPromoDlg 메시지 처리기

void CSearchPromoDlg::OnBnClickedButtonSearchpromo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	UpdateData(TRUE);


	char caaPromoID[MIN_TOKENS][MAX_LENGTH];
	int iPromoIDNum = 0;
	for (int i = 0; i < MIN_TOKENS; i++)
	{
		memset(caaPromoID[i], 0, sizeof(caaPromoID[i]));
	}
	CStringCSVToMultiArray(m_PromoID, caaPromoID, &iPromoIDNum);
	if (iPromoIDNum > 1)
	{
		EndWaitCursor();

		AfxMessageBox(_T("프로모션 아이디를 2개 이상 입력하면 안됩니다!"), MB_OK | MB_ICONERROR);
		return;
	}
	RemoveSpaces(caaPromoID[0]);


	m_lTotalRecNum = 0;
	if (!strcmp(caaPromoID[0], ""))
	{
		GetTotalPromoDocuNum(theApp.BigData_Promotions_collection, &m_lTotalRecNum);
	}
	else
	{
		GetPromoDocuNumByPromoID(theApp.BigData_Promotions_collection, caaPromoID[0], &m_lTotalRecNum);
	}
	m_pRPDR = (RegiPromoDlgRec*)malloc(sizeof(RegiPromoDlgRec) * m_lTotalRecNum);
	for (long i = 0; i < m_lTotalRecNum; i++)
	{
		m_pRPDR[i].caInsertedDate[0] = '\0';
		m_pRPDR[i].caPromoID[0] = '\0';
		m_pRPDR[i].caVisitorPattern[0] = '\0';
		for (int j = 0; j < MIN_TOKENS; j++)
		{
			m_pRPDR[i].caaArtIDs[j][0] = '\0';
			m_pRPDR[i].caaSearchKeywords[j][0] = '\0';
		}
	}


	GetRPDRByPromoID(theApp.BigData_Promotions_collection, caaPromoID[0], m_pRPDR, m_lTotalRecNum);

	InsertpRPDR();
	free(m_pRPDR);

	EndWaitCursor();

	this->Invalidate();
}

BOOL CSearchPromoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();   // 부모 클래스 초기화 (항상 호출해야 함)

	// TODO: 컨트롤 초기화 코드 작성
	// 예: 리스트 컨트롤 컬럼 추가
	// m_ListCtrl.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 60);

	// 리스트 컨트롤 초기 설정
	m_ListCtrlSearchedPromos.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 컬럼 헤더 추가 (5개)
	m_ListCtrlSearchedPromos.InsertColumn(0, _T("생성일"), LVCFMT_LEFT, 250);
	m_ListCtrlSearchedPromos.InsertColumn(1, _T("프로모션"), LVCFMT_LEFT, 150);
	m_ListCtrlSearchedPromos.InsertColumn(2, _T("기사"), LVCFMT_LEFT, 300);
	m_ListCtrlSearchedPromos.InsertColumn(3, _T("검색어"), LVCFMT_LEFT, 300);
	m_ListCtrlSearchedPromos.InsertColumn(4, _T("방문패턴"), LVCFMT_LEFT, 500);

	return TRUE;
}

void CSearchPromoDlg::InsertpRPDR()
{
	m_ListCtrlSearchedPromos.DeleteAllItems();

	CString strInsertedDate, strPromoID, strArtIDs, strSearchKeywords, strVisitorPattern;

	for (long i = 0; i < m_lTotalRecNum; i++)
	{
		strInsertedDate = m_pRPDR[i].caInsertedDate;
		strPromoID = m_pRPDR[i].caPromoID;
		strVisitorPattern = m_pRPDR[i].caVisitorPattern;
		strArtIDs = m_pRPDR[i].caaArtIDs[0];
		for (int j = 1; j < m_pRPDR[i].iArtIDsNum; j++)
		{
			strArtIDs += ", ";
			strArtIDs += m_pRPDR[i].caaArtIDs[j];
		}
		strSearchKeywords = m_pRPDR[i].caaSearchKeywords[0];
		for (int j = 1; j < m_pRPDR[i].iSearchKeywordsNum; j++)
		{
			strSearchKeywords += ", ";
			strSearchKeywords += m_pRPDR[i].caaSearchKeywords[j];
		}

		int nIndex = m_ListCtrlSearchedPromos.InsertItem(i, strInsertedDate);  // 첫 번째 컬럼(ID)
		m_ListCtrlSearchedPromos.SetItemText(nIndex, 1, strPromoID);
		m_ListCtrlSearchedPromos.SetItemText(nIndex, 2, strArtIDs);
		m_ListCtrlSearchedPromos.SetItemText(nIndex, 3, strSearchKeywords);
		m_ListCtrlSearchedPromos.SetItemText(nIndex, 4, strVisitorPattern);
	}
}

void CSearchPromoDlg::OnBnClickedButtonRegipromo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	int nItem = m_ListCtrlSearchedPromos.GetNextItem(-1, LVNI_SELECTED); // 선택된 행 인덱스 가져오기

	if (nItem == -1) {
		EndWaitCursor();

		AfxMessageBox(_T("선택된 레코드가 없습니다."));
		return;
	}

	// 각 컬럼의 텍스트 가져오기
	//CString strID = m_ListCtrlSearchedPromos.GetItemText(nItem, 0);
	m_RegiPromoDlg.m_PromoID = m_ListCtrlSearchedPromos.GetItemText(nItem, 1);
	m_RegiPromoDlg.m_ArtIDs = m_ListCtrlSearchedPromos.GetItemText(nItem, 2);
	m_RegiPromoDlg.m_SearchKeywords = m_ListCtrlSearchedPromos.GetItemText(nItem, 3);
	m_RegiPromoDlg.m_VisitorPattern = m_ListCtrlSearchedPromos.GetItemText(nItem, 4);

	EndWaitCursor();

	m_RegiPromoDlg.DoModal();
}

void CSearchPromoDlg::OnBnClickedButtonDeletepromo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	int nItem = m_ListCtrlSearchedPromos.GetNextItem(-1, LVNI_SELECTED); // 선택된 행 인덱스 가져오기

	if (nItem == -1) {
		EndWaitCursor();

		AfxMessageBox(_T("선택된 레코드가 없습니다."));
		return;
	}

	EndWaitCursor();

	int nResponse = AfxMessageBox(_T("이 작업은 되돌릴 수 없습니다.\n해당 프로모션을 삭제하시겠습니까?"), MB_YESNO | MB_ICONQUESTION);
	if (nResponse == IDYES)
	{
		BeginWaitCursor();

		// ✅ 사용자가 "예"를 클릭한 경우
	// 각 컬럼의 텍스트 가져오기
		CString IDcaInsertedDate = m_ListCtrlSearchedPromos.GetItemText(nItem, 0);
		CString PromoID = m_ListCtrlSearchedPromos.GetItemText(nItem, 1);

		std::string strInsertedDate;
		std::string strPromoID;
		char caInsertedDate[MAX_LENGTH], caPromoID[MAX_LENGTH];
		memset(caInsertedDate, 0, sizeof(caInsertedDate)); memset(caPromoID, 0, sizeof(caPromoID));

		strInsertedDate = CStringToStdString(IDcaInsertedDate);
		strncpy_s(caInsertedDate, MAX_LENGTH, strInsertedDate.c_str(), _TRUNCATE);
		caInsertedDate[MAX_LENGTH - 1] = '\0';
		strPromoID = CStringToStdString(PromoID);
		strncpy_s(caPromoID, MAX_LENGTH, strPromoID.c_str(), _TRUNCATE);
		caPromoID[MAX_LENGTH - 1] = '\0';

		UpdateDocuByPromoIDArtIDsQueryKeywordsVisitings(theApp.BigData_Promotions_collection, caPromoID, \
			caInsertedDate, theApp.m_User);

		EndWaitCursor();

		AfxMessageBox(_T("해당 프로모션이 삭제되었습니다!"));
		return;
	}
	else if (nResponse == IDNO)
	{
		// ❌ 사용자가 "아니오"를 클릭한 경우
		AfxMessageBox(_T("해당 프로모션의 삭제가 취소되었습니다!"));
		return;
	}
}
