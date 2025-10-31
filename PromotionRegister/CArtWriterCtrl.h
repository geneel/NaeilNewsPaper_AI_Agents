#pragma once
#include <afxwin.h>


class CArtWriterCtrl :
    public CStatic
{
    DECLARE_DYNAMIC(CArtWriterCtrl)

public:
    CArtWriterCtrl();
    virtual ~CArtWriterCtrl();

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

