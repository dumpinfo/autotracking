// viewbuttonDoc.h : interface of the CViewbuttonDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWBUTTONDOC_H__751FD0A7_DF9C_4170_BB3E_07FFCB414B03__INCLUDED_)
#define AFX_VIEWBUTTONDOC_H__751FD0A7_DF9C_4170_BB3E_07FFCB414B03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CViewbuttonDoc : public CDocument
{
protected: // create from serialization only
	CViewbuttonDoc();
	DECLARE_DYNCREATE(CViewbuttonDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewbuttonDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CViewbuttonDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CViewbuttonDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWBUTTONDOC_H__751FD0A7_DF9C_4170_BB3E_07FFCB414B03__INCLUDED_)
