#pragma once


typedef ATL::CSimpleArray<CString> StringList;


class CSxSItem
{
public:
    BOOL    bIsIgnoreItem;
    CString strType;
    CString strName;
    CString strVersion;
    CString strKeyToken;
    CString strProcessor;

    StringList  listFiles;
};


__inline bool operator == (const CSxSItem& item1, const CSxSItem& item2)
{
    return 
        item1.bIsIgnoreItem == item2.bIsIgnoreItem
        && item1.strName == item2.strName
        && item1.strKeyToken == item2.strKeyToken
        && item1.strType == item2.strType
        && item1.strVersion == item2.strVersion
        && item1.strProcessor == item2.strProcessor;
}

typedef ATL::CSimpleArray<CSxSItem> SxSList;

class CSxSParser
{
public:
    CSxSParser(void);
    ~CSxSParser(void);

public:
    void    Clear();
    BOOL    AddFile(LPCTSTR szFilePath);

    BOOL    Reparse();
    const SxSList& GetSxSList() const;

    BOOL    Export(LPCTSTR szExportPath, CString& strMsg) const;

private:
    BOOL ParseFile(LPCTSTR szFilePath);
    BOOL ParseAssembly(const CStringA& strAssembly, CSxSItem& item) const;

    BOOL ExportFolder(LPCTSTR szTag, LPCTSTR szName, LPCTSTR szSrc, LPCTSTR szDst, CString& strMsg) const;
    BOOL ExportFile(LPCTSTR szTag, LPCTSTR szName, LPCTSTR szSrc, LPCTSTR szDst, CString& strMsg) const;

    StringList FindMatchedFSObjects(LPCTSTR szParentFolder, LPCTSTR szFilter, BOOL bIsFile) const;

    BOOL MakeEmptyFolder(LPCTSTR szFolder) const;

    BOOL AddAssemblyItem(const CSxSItem& item);

    BOOL IsIgnoreItem(const CSxSItem& item) const;

private:
    StringList  m_arrFiles;
    SxSList m_SxSList;
};
