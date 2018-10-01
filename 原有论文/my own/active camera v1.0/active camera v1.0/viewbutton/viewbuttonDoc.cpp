// viewbuttonDoc.cpp : implementation of the CViewbuttonDoc class
//

#include "stdafx.h"
#include "viewbutton.h"

#include "viewbuttonDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonDoc

IMPLEMENT_DYNCREATE(CViewbuttonDoc, CDocument)

BEGIN_MESSAGE_MAP(CViewbuttonDoc, CDocument)
	//{{AFX_MSG_MAP(CViewbuttonDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonDoc construction/destruction

CViewbuttonDoc::CViewbuttonDoc()
{
	// TODO: add one-time construction code here

}

CViewbuttonDoc::~CViewbuttonDoc()
{
}

BOOL CViewbuttonDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CViewbuttonDoc serialization

void CViewbuttonDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonDoc diagnostics

#ifdef _DEBUG
void CViewbuttonDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CViewbuttonDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonDoc commands
