// CSearchArtDlg.cpp: 구현 파일
//

#include "pch.h"
#include "PromotionRegister.h"
#include "afxdialogex.h"
#include "CSearchArtDlg.h"


// CSearchArtDlg 대화 상자

IMPLEMENT_DYNAMIC(CSearchArtDlg, CDialogEx)

CSearchArtDlg::CSearchArtDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SearchArt, pParent)
	, m_ArtURL(_T(""))
{

}

CSearchArtDlg::~CSearchArtDlg()
{
}

void CSearchArtDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ArtURL, m_ArtURL);
}


BEGIN_MESSAGE_MAP(CSearchArtDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ArtGraph, &CSearchArtDlg::OnBnClickedButtonArtgraph)
END_MESSAGE_MAP()


// CSearchArtDlg 메시지 처리기

void CSearchArtDlg::OnBnClickedButtonArtgraph()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    BeginWaitCursor();

	UpdateData(TRUE);


    char caURL[TIME_SLOT_SIZE / 2], caArtID[READ_BUF_SIZE];
    caURL[TIME_SLOT_SIZE / 2 - 1] = '\0';
    caArtID[READ_BUF_SIZE - 1] = '\0';
    std::string strURL = CStringToStdString(m_ArtURL);
    strncpy_s(caURL, TIME_SLOT_SIZE / 2, strURL.c_str(), _TRUNCATE);
    caURL[TIME_SLOT_SIZE / 2 - 1] = '\0';
    if (strcmp(caURL, ""))
    {
        int count = 0;
        char** parts = split_url_all(caURL, &count);
        for (int i = 0; i < count; i++) {
            if (!strcmp("read", parts[i]))
            {
                keep_digits(parts[i + 1]);
                strncpy_s(caArtID, READ_BUF_SIZE, parts[i + 1], _TRUNCATE);
                caArtID[READ_BUF_SIZE - 1] = '\0';
            }
            free(parts[i]); // 해제
        }
        free(parts);

        if (strcmp(caArtID, ""))
        {
            strncpy_s(m_ArtGraphDlg.caArtID, READ_BUF_SIZE, caArtID, _TRUNCATE);
            m_ArtGraphDlg.caArtID[READ_BUF_SIZE - 1] = '\0';
            m_ArtGraphDlg.m_ArtID = m_ArtGraphDlg.caArtID;

            ArticleDocu TheArticle;
            char caUtf8ArtID[READ_BUF_SIZE];
            std::string utf8_caArtID = CharArrayToUTF8(caArtID);
            strncpy_s(caUtf8ArtID, READ_BUF_SIZE, utf8_caArtID.c_str(), _TRUNCATE);
            caUtf8ArtID[READ_BUF_SIZE - 1] = '\0';
            int iRSLT = GetArticleDocuByArticleID(theApp.BigData_Articles_collection, caUtf8ArtID, &TheArticle);

            if (iRSLT == 0)
            {
                char utf8_Date[READ_BUF_SIZE], caDate[READ_BUF_SIZE];
                MongoISODateToString(TheArticle.RegisteredDate, utf8_Date, READ_BUF_SIZE);
                Utf8ToAnsi(utf8_Date, caDate, READ_BUF_SIZE);
                strncpy_s(m_ArtGraphDlg.caArtRegDate, READ_BUF_SIZE, caDate, _TRUNCATE);
                m_ArtGraphDlg.caArtRegDate[READ_BUF_SIZE - 1] = '\0';
                m_ArtGraphDlg.m_ArtRegDate = m_ArtGraphDlg.caArtRegDate;

                m_ArtGraphDlg.iTotalClassNum = TheArticle.ClassAxisDim;
                m_ArtGraphDlg.iTotalWriterNum = TheArticle.WriterAxisDim;
                for (int i = 0; i < TheArticle.ClassAxisDim; i++)
                {
                    m_ArtGraphDlg.dNormalClass[i] = TheArticle.NormalClass[i];
                }
                for (int i = 0; i < TheArticle.WriterAxisDim; i++)
                {
                    m_ArtGraphDlg.dNormalWriter[i] = TheArticle.NormalWriter[i];
                }
                for (int i = 0; i < EMB_SIZE; i++)
                {
                    m_ArtGraphDlg.dArticleEmb[i] = TheArticle.ArticleAccumEmbedding[i];
                    m_ArtGraphDlg.dQueryEmb[i] = TheArticle.QueryAccumEmbedding[i];
                }

                EndWaitCursor();

                m_ArtGraphDlg.DoModal();
            }
            else
            {
                EndWaitCursor();

                AfxMessageBox(_T("선택된 기사의 속성에 대한 분석이 아직 완료되지 않았습니다!"));
                return;
            }
        }
        else
        {
            EndWaitCursor();

            AfxMessageBox(_T("기사 URL 입력창에 잘못된 내용이 입력되었습니다!\n반드시 www.naeil.com의 기사 URL을 입력해야만 합니다!"));
            return;
        }

    }
    else
    {
        EndWaitCursor();

        AfxMessageBox(_T("기사 URL 입력창에 아무런 내용이 없습니다!"));
        return;
    }
}
