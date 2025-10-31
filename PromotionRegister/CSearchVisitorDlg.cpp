// CSearchVisitorDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CSearchVisitorDlg.h"


// CSearchVisitorDlg 대화 상자

IMPLEMENT_DYNAMIC(CSearchVisitorDlg, CDialogEx)

CSearchVisitorDlg::CSearchVisitorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SearchVisitor, pParent)
	, m_VisitorCID(_T(""))
	, m_VisitorName(_T(""))
	, m_VisitorPhone(_T(""))
	, m_VisitorEmail(_T(""))
	, m_VisitorID(_T(""))
	, m_VisitorRecommender(_T(""))
{
	m_pVisitors = NULL;
	m_VisitorNum = 0;
}

CSearchVisitorDlg::~CSearchVisitorDlg()
{
}

void CSearchVisitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_VisitorList, m_ListCtrlVisitorList);
	DDX_Text(pDX, IDC_EDIT_VisitorCID, m_VisitorCID);
	DDX_Text(pDX, IDC_EDIT_VisitorName, m_VisitorName);
	DDX_Text(pDX, IDC_EDIT_VisitorPhone, m_VisitorPhone);
	DDX_Text(pDX, IDC_EDIT_VisitorEmail, m_VisitorEmail);
	DDX_Text(pDX, IDC_EDIT_VisitorID, m_VisitorID);
	DDX_Text(pDX, IDC_EDIT_VisitorRecommender, m_VisitorRecommender);
}


BEGIN_MESSAGE_MAP(CSearchVisitorDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SearchVisitor, &CSearchVisitorDlg::OnBnClickedButtonSearchvisitor)
	ON_BN_CLICKED(IDC_BUTTON_VisitorGraph, &CSearchVisitorDlg::OnBnClickedButtonVisitorgraph)
END_MESSAGE_MAP()


// CSearchVisitorDlg 메시지 처리기


BOOL CSearchVisitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();   // 부모 클래스 초기화 (항상 호출해야 함)

	// TODO: 컨트롤 초기화 코드 작성
	// 예: 리스트 컨트롤 컬럼 추가
	// m_ListCtrl.InsertColumn(0, _T("ID"), LVCFMT_LEFT, 60);

	// 리스트 컨트롤 초기 설정
	m_ListCtrlVisitorList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 컬럼 헤더 추가 (5개)
	m_ListCtrlVisitorList.InsertColumn(0, _T("고유키"), LVCFMT_LEFT, 200);
	m_ListCtrlVisitorList.InsertColumn(1, _T("성명"), LVCFMT_LEFT, 150);
	m_ListCtrlVisitorList.InsertColumn(2, _T("전화번호"), LVCFMT_LEFT, 200);
	m_ListCtrlVisitorList.InsertColumn(3, _T("e-mail"), LVCFMT_LEFT, 200);
	m_ListCtrlVisitorList.InsertColumn(4, _T("아이디"), LVCFMT_LEFT, 150);
	m_ListCtrlVisitorList.InsertColumn(4, _T("추천인"), LVCFMT_LEFT, 300);

	return TRUE;
}

void CSearchVisitorDlg::OnBnClickedButtonSearchvisitor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	UpdateData(TRUE);

	char caCID[NAME_SIZE], caName[NAME_SIZE], caPhone[NAME_SIZE], \
		caMail[NAME_SIZE], caID[NAME_SIZE], caRecommenders[NAME_SIZE];
	memset(caCID, 0, NAME_SIZE); memset(caName, 0, NAME_SIZE); memset(caPhone, 0, NAME_SIZE);
	memset(caMail, 0, NAME_SIZE); memset(caID, 0, NAME_SIZE); memset(caRecommenders, 0, NAME_SIZE);

	std::string  strCID = CStringToStdString(m_VisitorCID);
	strncpy_s(caCID, NAME_SIZE, strCID.c_str(), _TRUNCATE);
	caCID[NAME_SIZE - 1] = '\0';

	std::string  strName = CStringToStdString(m_VisitorName);
	strncpy_s(caName, NAME_SIZE, strName.c_str(), _TRUNCATE);
	caName[NAME_SIZE - 1] = '\0';

	std::string  strPhone = CStringToStdString(m_VisitorPhone);
	strncpy_s(caPhone, NAME_SIZE, strPhone.c_str(), _TRUNCATE);
	caPhone[NAME_SIZE - 1] = '\0';

	std::string  strMail = CStringToStdString(m_VisitorEmail);
	strncpy_s(caMail, NAME_SIZE, strMail.c_str(), _TRUNCATE);
	caMail[NAME_SIZE - 1] = '\0';

	std::string  strID = CStringToStdString(m_VisitorID);
	strncpy_s(caID, NAME_SIZE, strID.c_str(), _TRUNCATE);
	caID[NAME_SIZE - 1] = '\0';

	std::string  strRecommenders = CStringToStdString(m_VisitorRecommender);
	strncpy_s(caRecommenders, NAME_SIZE, strRecommenders.c_str(), _TRUNCATE);
	caRecommenders[NAME_SIZE - 1] = '\0';

	GetVisitorDocuNum(theApp.BigData_Visitors_collection, caCID, caName, caPhone, \
		caMail, caID, caRecommenders, &(m_VisitorNum));
	m_pVisitors = (VisitorDocu*)malloc(sizeof(VisitorDocu) * m_VisitorNum);
	GetVisitorDocu(theApp.BigData_Visitors_collection, caCID, caName, caPhone, \
		caMail, caID, caRecommenders, m_pVisitors);

	InsertpVisitors();
	free(m_pVisitors);

	EndWaitCursor();

	this->Invalidate();
}

void CSearchVisitorDlg::InsertpVisitors()
{
	m_ListCtrlVisitorList.DeleteAllItems();

	CString cstrVisitorCID, cstrVisitorName, cstrVisitorPhone, cstrVisitorEmail, cstrVisitorID, cstrVisitorRecommender;

	for (int i = 0; i < m_VisitorNum; i++)
	{
		cstrVisitorCID = m_pVisitors[i].caCID;
		cstrVisitorName = m_pVisitors[i].caName;
		cstrVisitorPhone = m_pVisitors[i].caPhone;
		cstrVisitorEmail = m_pVisitors[i].caMail;
		cstrVisitorID = m_pVisitors[i].caID;
		cstrVisitorRecommender = m_pVisitors[i].caRecommenders;

		int nIndex = m_ListCtrlVisitorList.InsertItem(i, cstrVisitorCID);  // 첫 번째 컬럼(ID)
		m_ListCtrlVisitorList.SetItemText(nIndex, 1, cstrVisitorName);
		m_ListCtrlVisitorList.SetItemText(nIndex, 2, cstrVisitorPhone);
		m_ListCtrlVisitorList.SetItemText(nIndex, 3, cstrVisitorEmail);
		m_ListCtrlVisitorList.SetItemText(nIndex, 4, cstrVisitorID);
		m_ListCtrlVisitorList.SetItemText(nIndex, 5, cstrVisitorRecommender);
	}
}

void CSearchVisitorDlg::OnBnClickedButtonVisitorgraph()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	int nItem = m_ListCtrlVisitorList.GetNextItem(-1, LVNI_SELECTED); // 선택된 행 인덱스 가져오기

	if (nItem == -1) {
		EndWaitCursor();

		AfxMessageBox(_T("선택된 레코드가 없습니다."));
		return;
	}

	// 첫 번째 컬럼의 cid 가져오기
	CString cstrCID = m_ListCtrlVisitorList.GetItemText(nItem, 0);
	std::string strCID = CStringToStdString(cstrCID);
	char caCID[READ_BUF_SIZE];
	strncpy_s(caCID, READ_BUF_SIZE, strCID.c_str(), _TRUNCATE);
	caCID[READ_BUF_SIZE - 1] = '\0';

	AnalDocu ThisAnal;
	int iRSLT = GetAnalDocuByCID(theApp.BigData_Anals_collection, caCID, &ThisAnal);

	if (iRSLT == 0)
	{
		strncpy_s(m_VisitorGraphDlg.caVisitorCID, READ_BUF_SIZE, ThisAnal.cid, _TRUNCATE);
		m_VisitorGraphDlg.caVisitorCID[READ_BUF_SIZE - 1] = '\0';
		strncpy_s(m_VisitorGraphDlg.caAnalRegDate, READ_BUF_SIZE, ThisAnal.RegistedDate, _TRUNCATE);
		m_VisitorGraphDlg.caAnalRegDate[READ_BUF_SIZE - 1] = '\0';

		m_VisitorGraphDlg.iTotalClassNum = ThisAnal.ClassAxisDim;
		m_VisitorGraphDlg.iTotalWriterNum = ThisAnal.WriterAxisDim;
		m_VisitorGraphDlg.iTotalEventSize = ThisAnal.TotalEventSize;

		for (int i = 0; i < m_VisitorGraphDlg.iTotalClassNum; i++)
		{
			m_VisitorGraphDlg.dNormalClass[i] = ThisAnal.NormalClass[i];
		}
		for (int i = 0; i < m_VisitorGraphDlg.iTotalWriterNum; i++)
		{
			m_VisitorGraphDlg.dNormalWriter[i] = ThisAnal.NormalWriter[i];
		}
		for (int i = 0; i < TIME_SLOT_SIZE / 2; i++)
		{
			m_VisitorGraphDlg.dPeriod[i] = ThisAnal.Period[i];
		}
		for (int i = 0; i < TIME_SLOT_SIZE; i++)
		{
			m_VisitorGraphDlg.dAccel[i] = ThisAnal.Acceleration[i];
		}
		for (int i = 0; i < EMB_SIZE; i++)
		{
			m_VisitorGraphDlg.dArticleEmb[i] = ThisAnal.ArticleAccumEmbedding[i];
			m_VisitorGraphDlg.dQueryEmb[i] = ThisAnal.QueryAccumEmbedding[i];
		}

		m_VisitorGraphDlg.m_VisitorCID = m_VisitorGraphDlg.caVisitorCID;
		m_VisitorGraphDlg.m_AnalRegDate = m_VisitorGraphDlg.caAnalRegDate;
		char caTotalEventSize[READ_BUF_SIZE];
		sprintf_s(caTotalEventSize, READ_BUF_SIZE, "%d", m_VisitorGraphDlg.iTotalEventSize);
		m_VisitorGraphDlg.m_TotalEventSize = caTotalEventSize;


		EndWaitCursor();

		m_VisitorGraphDlg.DoModal();
	}
	else
	{
		EndWaitCursor();

		AfxMessageBox(_T("선택된 방문고객의 속성에 대한 분석이 아직 완료되지 않았습니다!"));
		return;
	}
}
