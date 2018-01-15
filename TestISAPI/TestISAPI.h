#if !defined(AFX_TESTISAPI_H__3E6FE796_37B5_4F91_B187_1E7029934094__INCLUDED_)
#define AFX_TESTISAPI_H__3E6FE796_37B5_4F91_B187_1E7029934094__INCLUDED_

#include "resource.h"

class CTestISAPIExtension : public CHttpServer
{
public:
	CTestISAPIExtension();
	~CTestISAPIExtension();

	//{{AFX_VIRTUAL(CTestISAPIExtension)
	virtual BOOL GetExtensionVersion(HSE_VERSION_INFO* pVer);
	//}}AFX_VIRTUAL
	virtual BOOL TerminateExtension(DWORD dwFlags);

	void Default(CHttpServerContext* pCtxt);

	DECLARE_PARSE_MAP()

	//{{AFX_MSG(CTestISAPIExtension)
	//}}AFX_MSG
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTISAPI_H__3E6FE796_37B5_4F91_B187_1E7029934094__INCLUDED)
