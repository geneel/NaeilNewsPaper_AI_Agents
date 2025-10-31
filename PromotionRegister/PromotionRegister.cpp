
// PromotionRegister.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "PromotionRegister.h"
#include "PromotionRegisterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPromotionRegisterApp

BEGIN_MESSAGE_MAP(CPromotionRegisterApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPromotionRegisterApp 생성

CPromotionRegisterApp::CPromotionRegisterApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
    memset(caBigDataServerIP, 0, sizeof(caBigDataServerIP)); memset(caDBServerIP, 0, sizeof(caDBServerIP));
    memset(BigData_uri_string, 0, sizeof(BigData_uri_string)); memset(m_User, 0, sizeof(m_User));
    memset(DB_HOST, 0, sizeof(DB_HOST)); memset(DB_USER, 0, sizeof(DB_USER)); memset(DB_PASS, 0, sizeof(DB_PASS)); memset(DB_NAME, 0, sizeof(DB_NAME));

    BigData_uri = NULL;  BigData_client = NULL;  BigData_database = NULL;
    BigData_Visitors_collection = BigData_Anals_collection = BigData_Articles_collection = NULL;
    BigData_Promotions_collection = BigData_Users_collection = NULL;
    BigData_error = {0};
    DB_conn = NULL;
    DB_result = NULL;
    DB_connection = {0};
    DB_row = {0};
}


// 유일한 CPromotionRegisterApp 개체입니다.

CPromotionRegisterApp theApp;


// CPromotionRegisterApp 초기화

BOOL CPromotionRegisterApp::InitInstance()
{
	// Windows XP에서는 InitCommonControlsEx()를 필요로 합니다.
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 시작
    ////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MANUAL_DEBUG
    printf("This is starting point.\n");
#endif
    FILE* fptr;
    errno_t TmpErr= fopen_s(&fptr, "PromotionRegisterSetting.set", "rt");
    if (fptr == NULL)
    {
        printf("\nPromotionRegisterSetting.set File should be exist.\n");
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : PromotionRegisterSetting.set File should be exist."));
        return FALSE;
    }

    int rdcnt;
    rdcnt = fscanf_s(fptr, "%s", caBigDataServerIP);
    if (!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong BigDataServerIP in PromotionRegisterSetting.set\n");
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : Wrong BigDataServerIP in PromotionRegisterSetting.set"));
        return FALSE;
    }

    rdcnt = fscanf_s(fptr, "%s", caDBServerIP);
    if (!rdcnt)
    {
        fclose(fptr);
        printf("\nWrong DBServerIP in PromotionRegisterSetting.set\n");
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : Wrong DBServerIP in PromotionRegisterSetting.set"));
        return FALSE;
    }

    fclose(fptr);
#ifdef MANUAL_DEBUG
    printf("\nNaeil-CRM Big Data Server(for www.naeil.com) : IP = %s\n", caBigDataServerIP);
    printf("\nnaeil-msp-db Server(for www.naeil.com) : IP = %s\n", caDBServerIP);
#endif


    strcpy_s(BigData_uri_string, STR_SIZE, "");
    BigData_uri_string[STR_SIZE - 1] = '\0';
    strcat_s(BigData_uri_string, STR_SIZE - strlen(BigData_uri_string) - 1, caBigDataServerIP);
    strcat_s(BigData_uri_string, STR_SIZE - strlen(BigData_uri_string) - 1, "");
    strcat_s(BigData_uri_string, STR_SIZE - strlen(BigData_uri_string) - 1, "");

    //  11.10.2.33번 서버 : naeil-msp-db
    strcpy_s(DB_HOST, NAME_SIZE, caDBServerIP);    //호스트명
    DB_HOST[NAME_SIZE - 1] = '\0';
    strcpy_s(DB_USER, NAME_SIZE, "");    //사용자명
    DB_USER[NAME_SIZE - 1] = '\0';
    strcpy_s(DB_PASS, NAME_SIZE, "");    //비밀번호
    DB_PASS[NAME_SIZE - 1] = '\0';
    strcpy_s(DB_NAME, NAME_SIZE, "");    //DB명
    DB_NAME[NAME_SIZE - 1] = '\0';

    /*
    * Required to initialize libmongoc's internals
    */
    mongoc_init();
    /*
    * Safely create a MongoDB URI object from the given string
    */
    BigData_uri = mongoc_uri_new_with_error(BigData_uri_string, &BigData_error);
    if (!BigData_uri) {
        printf("failed to parse URI: %s\n"
            "error message:       %s\n",
            BigData_uri_string,
            BigData_error.message);
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : failed to parse URI"));
        fprintf(stderr,
            "failed to parse URI: %s\n"
            "error message:       %s\n",
            BigData_uri_string,
            BigData_error.message);
        return EXIT_FAILURE;
    }
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr is connected\n");
#endif
    /*
    * Create a new client instance
    */
    BigData_client = mongoc_client_new_from_uri(BigData_uri);
    if (!BigData_client) {
        printf("\nError : mongoc_client_new_from_uri\n");
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : Error : mongoc_client_new_from_uri"));
        return EXIT_FAILURE;
    }
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-Client is connected\n");
#endif
    /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples).
    */
    mongoc_client_set_appname(BigData_client, "PromotionRegister");
    /*
    * Get a handle on the database "db_name" and collection "coll_name"
    */
    BigData_database = mongoc_client_get_database(BigData_client, "WwwNaeilCom2023");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-DB is connected\n");
#endif
    BigData_Users_collection = mongoc_client_get_collection(BigData_client, "WwwNaeilCom2023", "Users");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-EventsCollection is connected\n");
#endif
    BigData_Visitors_collection = mongoc_client_get_collection(BigData_client, "WwwNaeilCom2023", "Visitors");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-VisitorsCollection is connected\n");
#endif
    BigData_Anals_collection = mongoc_client_get_collection(BigData_client, "WwwNaeilCom2023", "Anals");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-AnalsCollection is connected\n");
#endif
    BigData_Articles_collection = mongoc_client_get_collection(BigData_client, "WwwNaeilCom2023", "Articles");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-ArticlesCollection is connected\n");
#endif
    BigData_Promotions_collection = mongoc_client_get_collection(BigData_client, "WwwNaeilCom2023", "Promotions");
#ifdef MANUAL_DEBUG
    printf("\nSuccess : BigDataSvr-PromotionsCollection is connected\n");
#endif

    mysql_init(&DB_connection);
    DB_conn = mysql_real_connect(&DB_connection, DB_HOST, DB_USER, DB_PASS, DB_NAME, 13306, (char*)NULL, 0);
    if (DB_conn == NULL) {
        // ❌ 실패: 에러 메시지 출력
        fprintf(stderr, "\nMySQL 연결 실패: %s\n", mysql_error(&DB_connection));
        AfxMessageBox(_T("PromotionRegister.cpp-InitInstance() : MySQL 연결 실패"));
        return FALSE;
    }
    else {
#ifdef MANUAL_DEBUG
        // ✅ 성공
        printf("\nMySQL 연결 성공!\n");
#endif
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 접속 : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager *pShellManager = new CShellManager;

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

    // 1. 먼저 로그인 다이얼로그 실행
    CLoginDlg loginDlg;
    m_Login = FALSE;
    if (loginDlg.DoModal() != IDOK) 
    {
        return FALSE; // 로그인 실패/취소 → 프로그램 종료
    }
    if (!m_Login)
    {
        AfxMessageBox(_T("로그인에 실패했습니다!"), MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 2. 로그인 성공하면 메인 다이얼로그 실행
    CPromotionRegisterDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 대화 상자가 닫혔으므로 응용 프로그램의 메시지 펌프를 시작하지 않고 응용 프로그램을 끝낼 수 있도록 FALSE를
	// 반환합니다.

    return FALSE;
}


int CPromotionRegisterApp::ExitInstance()
{
    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 메모리 Clear : 시작    
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    mongoc_collection_destroy(BigData_Users_collection);
    mongoc_collection_destroy(BigData_Visitors_collection);
    mongoc_collection_destroy(BigData_Anals_collection);
    mongoc_collection_destroy(BigData_Articles_collection);
    mongoc_collection_destroy(BigData_Promotions_collection);
    mongoc_database_destroy(BigData_database);
    mongoc_uri_destroy(BigData_uri);
    mongoc_client_destroy(BigData_client);
    mongoc_cleanup();

    mysql_close(DB_conn);
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //  BigDataServer, naeil-msp-db 메모리 Clear : 끝    
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    return CWinApp::ExitInstance();
}
