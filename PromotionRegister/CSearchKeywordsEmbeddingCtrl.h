#pragma once
#include <afxwin.h>


class CSearchKeywordsEmbeddingCtrl :
    public CStatic
{
    DECLARE_DYNAMIC(CSearchKeywordsEmbeddingCtrl)

public:
    CSearchKeywordsEmbeddingCtrl();
    virtual ~CSearchKeywordsEmbeddingCtrl();

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

