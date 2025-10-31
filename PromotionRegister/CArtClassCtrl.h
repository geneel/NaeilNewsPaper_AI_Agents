#pragma once
#include <afxwin.h>


class CArtClassCtrl :
    public CStatic
{
    DECLARE_DYNAMIC(CArtClassCtrl)

public:
    CArtClassCtrl();
    virtual ~CArtClassCtrl();

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();

public:
    void SetDataPtr(const double* pData, int dataCount);
    void DrawGraph(CDC* pDC, const CRect& rcClient);

private:
    const double* m_pData = nullptr;
    int m_nDataCount = 0;
};

