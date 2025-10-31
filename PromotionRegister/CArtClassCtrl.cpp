#include "pch.h"
#include "CArtClassCtrl.h"


IMPLEMENT_DYNAMIC(CArtClassCtrl, CStatic)

BEGIN_MESSAGE_MAP(CArtClassCtrl, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

CArtClassCtrl::CArtClassCtrl() {}
CArtClassCtrl::~CArtClassCtrl() {}

void CArtClassCtrl::SetDataPtr(const double* pData, int dataCount)
{
    m_pData = pData;
    m_nDataCount = dataCount;
    Invalidate(); // �ٽ� �׸��� ��û
}

void CArtClassCtrl::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    // ��� ���
    dc.FillSolidRect(rc, RGB(255, 255, 255));

    // �׷��� ����: sin ����
    DrawGraph(&dc, rc);
}

void CArtClassCtrl::DrawGraph(CDC* pDC, const CRect& rc)
{
    // �����ǥ ��ȯ ����
    auto X = [&](double relX) { return rc.left + int(relX * rc.Width()); };
    auto Y = [&](double relY) { return rc.bottom - int(relY * rc.Height()); };

    // Y�� �ڵ� �����ϸ�: ������ �ּ�/�ִ� Ž��
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