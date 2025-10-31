#include "pch.h"
#include "CSearchKeywordsEmbeddingCtrl.h"


IMPLEMENT_DYNAMIC(CSearchKeywordsEmbeddingCtrl, CStatic)

BEGIN_MESSAGE_MAP(CSearchKeywordsEmbeddingCtrl, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CSearchKeywordsEmbeddingCtrl::CSearchKeywordsEmbeddingCtrl() {}
CSearchKeywordsEmbeddingCtrl::~CSearchKeywordsEmbeddingCtrl() {}

void CSearchKeywordsEmbeddingCtrl::SetDataPtr(const double* pData, int dataCount)
{
    m_pData = pData;
    m_nDataCount = dataCount;
    Invalidate(); // 다시 그리기 요청
}

void CSearchKeywordsEmbeddingCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    // 배경 흰색
    dc.FillSolidRect(rc, RGB(255, 255, 255));

    // 그래프 예시: sin 파형
    DrawGraph(&dc, rc);
}

void CSearchKeywordsEmbeddingCtrl::DrawGraph(CDC* pDC, const CRect& rc)
{
    // 상대좌표 변환 람다
    auto X = [&](double relX) { return rc.left + int(relX * rc.Width()); };
    auto Y = [&](double relY) { return rc.bottom - int(relY * rc.Height()); };

    // Y값 자동 스케일링: 데이터 최소/최대 탐색
    double minY = DBL_MAX, maxY = -DBL_MAX;
    for (int i = 0; i < m_nDataCount; ++i)
    {
        minY = min(minY, m_pData[i]);
        maxY = max(maxY, m_pData[i]);
    }
    double rangeY = (maxY - minY);
    if (rangeY < 1e-12) rangeY = 1.0;

    CPen pen(PS_SOLID, 2, RGB(0, 0, 255));
    CPen* oldPen = pDC->SelectObject(&pen);

    for (int i = 1; i < m_nDataCount; ++i)
    {
        double relX1 = (i - 1) / (double)(m_nDataCount - 1);
        double relX2 = i / (double)(m_nDataCount - 1);

        double normY1 = (m_pData[i - 1] - minY) / rangeY;
        double normY2 = (m_pData[i] - minY) / rangeY;

        pDC->MoveTo(X(relX1), Y(normY1));
        pDC->LineTo(X(relX2), Y(normY2));
    }

    pDC->SelectObject(oldPen);
}