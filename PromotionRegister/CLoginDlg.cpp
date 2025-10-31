// CLoginDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CLoginDlg.h"


// CLoginDlg 대화 상자

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_Login, pParent)
	, m_UserID(_T(""))
	, m_UserPassWord(_T(""))
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_UserID, m_UserID);
	DDX_Text(pDX, IDC_EDIT_UserPassWord, m_UserPassWord);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_Login, &CLoginDlg::OnBnClickedButtonLogin)
END_MESSAGE_MAP()


// CLoginDlg 메시지 처리기

void CLoginDlg::OnBnClickedButtonLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	UpdateData(TRUE);

	if ((m_UserID.GetLength() > (READ_BUF_SIZE - 1))|| (m_UserPassWord.GetLength() > (READ_BUF_SIZE - 1)))
	{
		theApp.m_Login = FALSE;
		OnOK();
	}

	// CString → char 배열 변환
	CT2CA pszConvertedAnsiString(m_UserID);
	const char* szStr = pszConvertedAnsiString;
	// 안전 복사 (_TRUNCATE: 너무 길면 잘라냄)
	strncpy_s(theApp.m_User, READ_BUF_SIZE, szStr, _TRUNCATE);
	theApp.m_User[READ_BUF_SIZE - 1] = '\0';

	char HPWD[NAME_SIZE];
	std::string UserPassWord = CStringToUtf8(m_UserPassWord);
	std::string HashedPWD = HashPassword(UserPassWord);
	strcpy_s(HPWD, sizeof(HPWD), HashedPWD.c_str());  // 안전하게 복사

	int iRecordNum;
	GetRecordNumByIdPwd(theApp.BigData_Users_collection, theApp.m_User, HPWD, &iRecordNum);

	if (iRecordNum == 1)
	{
		theApp.m_Login = TRUE;
	}
	else
	{
		theApp.m_Login = FALSE;
	}

	EndWaitCursor();

	OnOK();
}
