#include "StdAfx.h"
#include "PEResource.h"

CPEResource::CPEResource()
{
    m_hModule = NULL;
    m_hManifest = NULL;
}

CPEResource::~CPEResource(void)
{
    Close();
}

BOOL CPEResource::Open(CString strPEPath)
{
    Close();

    m_strPEPath = strPEPath;
    // m_hModule = ::LoadLibrary(strPEPath);
    m_hModule = ::LoadLibraryEx(strPEPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
    return (m_hModule != NULL);
}

BOOL CPEResource::GetManifest(LPVOID& pManifest, DWORD& dwLength)
{
    if(m_hManifest == NULL)
    {
        int nPos = m_strPEPath.ReverseFind(_T('.'));
        CString strExt;
        if(nPos != -1)
            strExt = m_strPEPath.Mid(nPos);
        strExt.MakeLower();
        if(strExt == _T(".exe"))
            m_hManifest = ::FindResource(m_hModule, MAKEINTRESOURCE(1), RT_MANIFEST);
        else
            m_hManifest = ::FindResource(m_hModule, MAKEINTRESOURCE(2), RT_MANIFEST);
    }
    if(m_hManifest == NULL)
        return FALSE;

    HGLOBAL hGlobal = ::LoadResource(m_hModule, m_hManifest);
    if(hGlobal == NULL)
        return FALSE;

    pManifest = ::LockResource(hGlobal);
    dwLength = ::SizeofResource(m_hModule, m_hManifest);
    return (pManifest != NULL);
}

void CPEResource::Close()
{
    if(m_hManifest != NULL)
    {
        ::FreeResource(m_hManifest);
        m_hManifest = NULL;
    }
    if(m_hModule != NULL)
    {
        ::FreeLibrary(m_hModule);
        m_hModule = NULL;
    }
}
