// CRegiPromoDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CRegiPromoDlg.h"

#include "FuncLib.h"



// CRegiPromoDlg 대화 상자

IMPLEMENT_DYNAMIC(CRegiPromoDlg, CDialogEx)

CRegiPromoDlg::CRegiPromoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_RegiPromo, pParent)
	, m_PromoID(_T(""))
	, m_ArtIDs(_T(""))
	, m_SearchKeywords(_T(""))
	, m_VisitorPattern(_T(""))
{

}

CRegiPromoDlg::~CRegiPromoDlg()
{
}

void CRegiPromoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PromoID, m_PromoID);
	DDX_Text(pDX, IDC_EDIT_ArtIDs, m_ArtIDs);
	DDX_Text(pDX, IDC_EDIT_SeearchKeywords, m_SearchKeywords);
	DDX_Text(pDX, IDC_EDIT_VisitorPattern, m_VisitorPattern);
}


BEGIN_MESSAGE_MAP(CRegiPromoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_PromoGraph, &CRegiPromoDlg::OnBnClickedButtonPromograph)
END_MESSAGE_MAP()


// CRegiPromoDlg 메시지 처리기

void CRegiPromoDlg::OnBnClickedButtonPromograph()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();

	UpdateData(TRUE);
	


	char caaPromoID[MIN_TOKENS][MAX_LENGTH], caaArtIDs[MIN_TOKENS][MAX_LENGTH], \
		caaSearchKeywords[MIN_TOKENS][MAX_LENGTH], caaVisitorPattern[MAX_TOKENS][MAX_LENGTH];
	int iPromoIDNum, iArtIDsNum, iSearchKeywordsNum, iVisitorPatternNum;
	for (int i = 0; i < MIN_TOKENS; i++)
	{
		memset(caaPromoID[i], 0, sizeof(caaPromoID[i])); memset(caaArtIDs[i], 0, sizeof(caaArtIDs[i]));
		memset(caaSearchKeywords[i], 0, sizeof(caaSearchKeywords[i])); memset(caaVisitorPattern[i], 0, sizeof(caaVisitorPattern[i]));
	}
	iPromoIDNum = iArtIDsNum = iSearchKeywordsNum = iVisitorPatternNum = 0;

	CStringCSVToMultiArray(m_PromoID, caaPromoID, &iPromoIDNum);
	CStringCSVToMultiArray(m_ArtIDs, caaArtIDs, &iArtIDsNum);
	CStringCSVToMultiArray(m_SearchKeywords, caaSearchKeywords, &iSearchKeywordsNum);
	CStringCSVToMultiArray(m_VisitorPattern, caaVisitorPattern, &iVisitorPatternNum);

	if (iPromoIDNum != 1)
	{
		EndWaitCursor();

		AfxMessageBox(_T("프로모션 아이디는 반드시 한 개를 입력해야 합니다!"), MB_OK | MB_ICONERROR);
		return;
	}
	if (iVisitorPatternNum > 2048)
	{
		EndWaitCursor();

		AfxMessageBox(_T("방문고객의 방문패턴은 2048개까지만 입력 가능합니다!"), MB_OK | MB_ICONERROR);
		return;
	}

	RemoveSpaces(caaPromoID[0]);
	for (int i = 0; i < iArtIDsNum; i++)
	{
		RemoveSpaces(caaArtIDs[i]);
	}
	for (int i = 0; i < iVisitorPatternNum; i++)
	{
		RemoveSpaces(caaVisitorPattern[i]);
	}

	Dictionary* pDict;
	pDict = (Dictionary*)malloc(sizeof(Dictionary) * iVisitorPatternNum);
	for (int i = 0; i < iVisitorPatternNum; i++)
	{
		ParseKeyValue(caaVisitorPattern[i], &(pDict[i]));
	}
	for (int i = 0; i < iVisitorPatternNum; i++)
	{
		if (atoi(pDict[i].Key) < 0)
		{
			EndWaitCursor();

			AfxMessageBox(_T("방문고객의 방문일자는 음수가 될 수 없습니다!"), MB_OK | MB_ICONERROR);
			return;
		}
		if (atoi(pDict[i].Value) < 0)
		{
			EndWaitCursor();

			AfxMessageBox(_T("방문고객의 방문횟수는 음수가 될 수 없습니다!"), MB_OK | MB_ICONERROR);
			return;
		}
		if (atoi(pDict[i].Key) > 2047)
		{
			EndWaitCursor();

			AfxMessageBox(_T("방문고객의 방문일자는 2047보다 클 수 없습니다!"), MB_OK | MB_ICONERROR);
			return;
		}
	}

	int iTmpNum = 0;
	GetRecNum(&(theApp.DB_connection), theApp.DB_conn, \
		theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, \
		"naeil_wms_db.WMS_ADBANNER", "BNNR_SEQ", caaPromoID[0], &iTmpNum);
	if (iTmpNum != 1)
	{
		EndWaitCursor();

		AfxMessageBox(_T("존재하지 않는 프로모션 아이디입니다!"), MB_OK | MB_ICONERROR);
		return;
	}
	for (int i = 0; i < iArtIDsNum; i++)
	{
		GetRecNum(&(theApp.DB_connection), theApp.DB_conn, \
			theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, \
			"naeil_wms_db.WMS_ARTICLE", "ART_ID", caaArtIDs[i], &iTmpNum);
		if (iTmpNum != 1)
		{
			EndWaitCursor();

			AfxMessageBox(_T("존재하지 않는 기사 아이디가 있습니다!"), MB_OK | MB_ICONERROR);
			return;
		}
	}



	m_PromoGraphDlg.m_PromoID = m_PromoID;
	m_PromoGraphDlg.m_ArtIDs = m_ArtIDs;
	m_PromoGraphDlg.m_SearchKeywords = m_SearchKeywords;

	GetDatesByPromoID(&(theApp.DB_connection), theApp.DB_conn, \
		theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, "naeil_wms_db.WMS_ADBANNER", "BNNR_SEQ", caaPromoID[0], \
		m_PromoGraphDlg.caPromoRegDate, READ_BUF_SIZE, m_PromoGraphDlg.caPromoStartDate, READ_BUF_SIZE, m_PromoGraphDlg.caPromoEndDate, READ_BUF_SIZE);
	m_PromoGraphDlg.m_PromoRegDate = Utf8ToCString(m_PromoGraphDlg.caPromoRegDate);
	m_PromoGraphDlg.m_PromoStartDate = Utf8ToCString(m_PromoGraphDlg.caPromoStartDate);
	m_PromoGraphDlg.m_PromoEndDate = Utf8ToCString(m_PromoGraphDlg.caPromoEndDate);

	strncpy_s(m_PromoGraphDlg.caPromoID, READ_BUF_SIZE, caaPromoID[0], _TRUNCATE);
	m_PromoGraphDlg.caPromoID[READ_BUF_SIZE - 1] = '\0';

	strncpy_s(m_PromoGraphDlg.caVisitorPattern, TEXT_SIZE * 10, CStringToUtf8(m_VisitorPattern).c_str(), _TRUNCATE);
	m_PromoGraphDlg.caVisitorPattern[TEXT_SIZE * 10 - 1] = '\0';

	m_PromoGraphDlg.iArtIDsNum = iArtIDsNum;
	m_PromoGraphDlg.iSearchKeywordsNum = iSearchKeywordsNum;
	for (int i = 0; i < iArtIDsNum; i++)
	{
		strncpy_s(m_PromoGraphDlg.caaArtIDs[i], MAX_LENGTH, caaArtIDs[i], _TRUNCATE);
		m_PromoGraphDlg.caaArtIDs[i][MAX_LENGTH - 1] = '\0';
	}
	for (int i = 0; i < iSearchKeywordsNum; i++)
	{
		strncpy_s(m_PromoGraphDlg.caaSearchKeywords[i], MAX_LENGTH, caaSearchKeywords[i], _TRUNCATE);
		m_PromoGraphDlg.caaSearchKeywords[i][MAX_LENGTH - 1] = '\0';
	}



	long iTotalClassNum, iTotalWriterNum;
	iTotalClassNum = iTotalWriterNum = 0;
	GetTotalRecNum(&(theApp.DB_connection), (theApp.DB_conn), (theApp.DB_HOST), (theApp.DB_USER), (theApp.DB_PASS), 
		(theApp.DB_NAME), "naeil_wms_db.WMS_CODE_CLASS", &iTotalClassNum);
	GetTotalRecNum(&(theApp.DB_connection), (theApp.DB_conn), (theApp.DB_HOST), (theApp.DB_USER), (theApp.DB_PASS), \
		(theApp.DB_NAME), "naeil_wms_db.WMS_USER", &iTotalWriterNum);
	m_PromoGraphDlg.iTotalClassNum = (int)iTotalClassNum;
	m_PromoGraphDlg.iTotalWriterNum = (int)iTotalWriterNum;



	char** caaClassAxis, ** caaWriterAxis;
	char caClassAxis[TEXT_SIZE], caWriterAxis[TEXT_SIZE];
	double *dpNormalClass, *dpNormalWriter;
	double dTime[TIME_SLOT_SIZE];

	caaClassAxis = (char**)malloc(sizeof(char*) * iTotalClassNum);
	for (int i = 0; i < iTotalClassNum; i++)
	{
		caaClassAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
		caaClassAxis[i][0] = '\0';
	}
	caaWriterAxis = (char**)malloc(sizeof(char*) * iTotalWriterNum);
	for (int i = 0; i < iTotalWriterNum; i++)
	{
		caaWriterAxis[i] = (char*)malloc(sizeof(char) * READ_BUF_SIZE);
		caaWriterAxis[i][0] = '\0';
	}
	caClassAxis[0] = '\0'; caWriterAxis[0] = '\0';
	dpNormalClass = (double*)malloc(sizeof(double) * iTotalClassNum);
	dpNormalWriter = (double*)malloc(sizeof(double) * iTotalWriterNum);
	for (int j = 0; j < iTotalClassNum; j++)
	{
		dpNormalClass[j] = 0;
	}
	for (int j = 0; j < iTotalWriterNum; j++)
	{
		dpNormalWriter[j] = 0;
	}
	for (int i = 0; i < TIME_SLOT_SIZE / 2; i++)
	{
		m_PromoGraphDlg.dPeriod[i] = m_PromoGraphDlg.dAccel[i * 2] = m_PromoGraphDlg.dAccel[i * 2 + 1] = dTime[i * 2] = dTime[i * 2 + 1] = 0;
	}



	GetAxis(&(theApp.DB_connection), theApp.DB_conn, theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, \
		"CODE_ID", "naeil_wms_db.WMS_CODE_CLASS", iTotalClassNum, caaClassAxis, READ_BUF_SIZE);
#ifdef MANUAL_DEBUG
	FILE* fp = NULL;
	errno_t err = fopen_s(&fp, "Out2.txt", "w");
	if (err != 0 || fp == NULL) {
		printf("파일 열기 실패 (err=%d)\n", err);
		EndWaitCursor();

		AfxMessageBox(_T("CRegiPromoDlg.cpp-OnBnClickedButtonPromograph() : 파일 열기 실패"));
		return FALSE;
	}
	for (int j = 0; j < iTotalClassNum; j++) {
		if (caaClassAxis[j] != NULL && caaClassAxis[j][0] != '\0') {
			fprintf(fp, "caaClassAxis[%d] : %s\n", j, caaClassAxis[j]);
		}
		else {
			fprintf(fp, "caaClassAxis[%d] : (null or empty)\n", j);
		}
	}
	fclose(fp);
#endif
	for (int i = 0; i < iTotalClassNum; i++)
	{
		int len = (int)strlen(caClassAxis);
		int remain = TEXT_SIZE - len;   // 남은 버퍼 크기
		if (remain > 0) {
			sprintf_s(caClassAxis + len, remain, "%s, ", caaClassAxis[i]);
		}
	}
	GetAxis(&(theApp.DB_connection), theApp.DB_conn, theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, \
		"USER_ID", "naeil_wms_db.WMS_USER", iTotalWriterNum, caaWriterAxis, READ_BUF_SIZE);
	for (int i = 0; i < iTotalWriterNum; i++)
	{
		int len = (int)strlen(caWriterAxis);
		int remain = TEXT_SIZE - len;   // 남은 버퍼 크기
		if (remain > 0) {
			sprintf_s(caWriterAxis + len, remain, "%s, ", caaWriterAxis[i]);
		}
	}

	strncpy_s(m_PromoGraphDlg.caClassAxis, TEXT_SIZE, caClassAxis, _TRUNCATE);
	m_PromoGraphDlg.caClassAxis[TEXT_SIZE - 1] = '\0';
	strncpy_s(m_PromoGraphDlg.caWriterAxis, TEXT_SIZE, caWriterAxis, _TRUNCATE);
	m_PromoGraphDlg.caWriterAxis[TEXT_SIZE - 1] = '\0';



	for (int i = 0; i < iVisitorPatternNum; i++)
	{
		dTime[TIME_SLOT_SIZE - 1 - atoi(pDict[i].Key)] = atoi(pDict[i].Value);
	}





	for (int i = 0; i < iArtIDsNum; i++)
	{
		int iClassIdx, iWriterIdx;
		iClassIdx = iWriterIdx = 0;
		
		GetClassIdxWriterIdxFromArtID(&(theApp.DB_connection), theApp.DB_conn, theApp.DB_HOST, theApp.DB_USER, theApp.DB_PASS, theApp.DB_NAME, \
			caaClassAxis, caaWriterAxis, iTotalClassNum, iTotalWriterNum, \
			caaArtIDs[i], &iClassIdx, &iWriterIdx);

		if (iClassIdx >= 0) dpNormalClass[iClassIdx]++;
		if (iWriterIdx >= 0) dpNormalWriter[iWriterIdx]++;
	}
	Normalize(dpNormalClass, iTotalClassNum);
	Normalize(dpNormalWriter, iTotalWriterNum);
	for (int i = 0; i < iTotalClassNum; i++)
	{
		m_PromoGraphDlg.dNormalClass[i] = dpNormalClass[i];
	}
	for (int i = 0; i < iTotalWriterNum; i++)
	{
		m_PromoGraphDlg.dNormalWriter[i] = dpNormalWriter[i];
	}



	for (int i = 0; i < iVisitorPatternNum; i++)
	{
		int len = (int)strlen(m_PromoGraphDlg.caVisitings);
		int remain = (TEXT_SIZE * 8) - len;   // 남은 버퍼 크기
		if (remain > 0) {
			sprintf_s(m_PromoGraphDlg.caVisitings + len, remain, "%s:", pDict[i].Key);
		}

		len = (int)strlen(m_PromoGraphDlg.caVisitings);
		remain = (TEXT_SIZE * 8) - len;   // 남은 버퍼 크기
		if (remain > 0) {
			sprintf_s(m_PromoGraphDlg.caVisitings + len, remain, "%s, ", pDict[i].Value);
		}
	}
	m_PromoGraphDlg.caVisitings[strlen(m_PromoGraphDlg.caVisitings) - 2] = '\0';



	double dLPFedTime[TIME_SLOT_SIZE], dFFT[TIME_SLOT_SIZE * 2 + 1], dPSD[TIME_SLOT_SIZE];
	gaussian_smooth_1d(dTime, dLPFedTime, TIME_SLOT_SIZE, SIGMA);
	for (int i = 0; i < TIME_SLOT_SIZE; i++)
	{
		dFFT[i * 2 + 1] = dLPFedTime[i];
		dFFT[i * 2 + 2] = 0;
	}
	four1(dFFT, TIME_SLOT_SIZE, 1);
	PowerSpectralDensity(dFFT, TIME_SLOT_SIZE, dPSD);
	for (int i = 0; i < TIME_SLOT_SIZE / 2; i++)
	{
		m_PromoGraphDlg.dPeriod[TIME_SLOT_SIZE / 2 - 1 - i] = dPSD[i + 1];
	}
	Normalize(m_PromoGraphDlg.dPeriod, TIME_SLOT_SIZE / 2);

	double dLPFilter[DWT_KLEN], dHPFilter[DWT_KLEN];
	dLPFilter[0] = 0.0; dLPFilter[1] = -0.000423989; dLPFilter[2] = 0.00110237;
	dLPFilter[3] = -0.00404203; dLPFilter[4] = 0.0363783; dLPFilter[5] = 0.254648;
	dLPFilter[6] = 0.424413; dLPFilter[7] = 0.254648; dLPFilter[8] = 0.0363783;
	dLPFilter[9] = -0.00404203; dLPFilter[10] = 0.00110237; dLPFilter[11] = -0.000423989;
	dHPFilter[0] = 0.0; dHPFilter[1] = 0.0; dHPFilter[2] = 0.0;
	dHPFilter[3] = -0.092042; dHPFilter[4] = -0.337216; dHPFilter[5] = -1.212978;
	dHPFilter[6] = -1.915714; dHPFilter[7] = 1.915714; dHPFilter[8] = 1.212978;
	dHPFilter[9] = 0.337216; dHPFilter[10] = 0.092042; dHPFilter[11] = 0.0;
	int iLevel = 0;
	GetDyadicLevel(TIME_SLOT_SIZE, &iLevel);
	double** dppOutput = (double**)malloc(sizeof(double*) * iLevel);
	for (int i = 0; i < iLevel; i++)
	{
		dppOutput[i] = (double*)malloc(sizeof(double) * TIME_SLOT_SIZE);
		for (int j = 0; j < TIME_SLOT_SIZE; j++)
		{
			dppOutput[i][j] = 0;
		}
	}
	DWTBank(dLPFilter, DWT_KLEN, dHPFilter, DWT_KLEN, iLevel, \
		dTime, TIME_SLOT_SIZE, dppOutput);
	GetAcceleration(dppOutput, TIME_SLOT_SIZE, iLevel, m_PromoGraphDlg.dAccel);
	Normalize(m_PromoGraphDlg.dAccel, TIME_SLOT_SIZE);
	for (int i = 0; i < iLevel; i++)
	{
		free(dppOutput[i]);
	}
	free(dppOutput);



	free(pDict);
	for (int i = 0; i < iTotalClassNum; i++)
	{
		free(caaClassAxis[i]);
	}
	for (int i = 0; i < iTotalWriterNum; i++)
	{
		free(caaWriterAxis[i]);
	}
	free(caaClassAxis);
	free(caaWriterAxis);
	free(dpNormalClass);
	free(dpNormalWriter);



	GetEmbsByArtIDsQueryKeywords(theApp.BigData_Promotions_collection, \
		m_PromoGraphDlg.caaArtIDs, m_PromoGraphDlg.iArtIDsNum, m_PromoGraphDlg.caaSearchKeywords, m_PromoGraphDlg.iSearchKeywordsNum, \
		m_PromoGraphDlg.dArticleEmb, m_PromoGraphDlg.dQueryEmb);



	EndWaitCursor();

	//m_PromoGraphDlg.Create(IDD_DIALOG_PromoGraph, this);
	//m_PromoGraphDlg.ShowWindow(SW_SHOW);
	m_PromoGraphDlg.DoModal();
}
