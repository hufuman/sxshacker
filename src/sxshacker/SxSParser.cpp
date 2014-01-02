#include "StdAfx.h"
#include "SxSParser.h"


#include "PEResource.h"
#include "Util.h"
#include "resource.h"

CSxSParser::CSxSParser(void)
{
}

CSxSParser::~CSxSParser(void)
{
}

void CSxSParser::Clear()
{
    m_SxSList.RemoveAll();
}

BOOL CSxSParser::AddFile(LPCTSTR szFilePath)
{
    return ParseFile(szFilePath);
}

const SxSList& CSxSParser::GetSxSList() const
{
    return m_SxSList;
}

BOOL CSxSParser::ParseFile(LPCTSTR szFilePath)
{
    CPEResource res;
    if(!res.Open(szFilePath))
        return FALSE;

    LPVOID pData = NULL;
    DWORD dwLength = 0;
    if(!res.GetManifest(pData, dwLength))
        return FALSE;

    CStringA strManifest;
    LPSTR pManifest = strManifest.GetBufferSetLength(dwLength);
    memcpy(pManifest, pData, dwLength);
    strManifest.ReleaseBuffer();

    int nStart = 0;
    CStringA strAssembly = Util::GetToken(strManifest, "<dependentAssembly>", "</dependentAssembly>", nStart);
    for(; !strAssembly.IsEmpty(); strAssembly = Util::GetToken(strManifest, "<dependentAssembly>", "</dependentAssembly>", nStart))
    {
        CSxSItem item;

        if(!ParseAssembly(strAssembly, item))
            return FALSE;

        item.listFiles.Add(szFilePath);
        AddAssemblyItem(item);
    }

    return TRUE;
}

BOOL CSxSParser::ParseAssembly(const CStringA& strAssembly, CSxSItem& item) const
{
    item.strType = Util::GetToken(strAssembly, "type=\"", "\"");
    item.strName = Util::GetToken(strAssembly, "name=\"", "\"");
    item.strVersion = Util::GetToken(strAssembly, "version=\"", "\"");
    item.strKeyToken = Util::GetToken(strAssembly, "publicKeyToken=\"", "\"");
    item.strProcessor = Util::GetToken(strAssembly, "processorArchitecture=\"", "\"");

    item.bIsIgnoreItem = IsIgnoreItem(item);

    return !item.strType.IsEmpty()
        && !item.strName.IsEmpty()
        && !item.strVersion.IsEmpty()
        && !item.strKeyToken.IsEmpty()
        && !item.strProcessor.IsEmpty();
}

BOOL CSxSParser::Export(LPCTSTR szExportPath, CString& strMsg) const
{
    CString strExportPath(szExportPath);
    if(strExportPath[strExportPath.GetLength() - 1] != _T('\\'))
        strExportPath += _T('\\');

    const SxSList& list = m_SxSList;

    CString strSxSFolder, strSxSFile, strTag, strDest;
    UINT uCount = list.GetSize();
    for(UINT i=0; i<uCount; ++ i)
    {
        const CSxSItem& item = m_SxSList[i];

        if(item.bIsIgnoreItem)
        {
            // no need to export assembly for common-controls
            continue;
        }

        strTag = item.strProcessor;
        strTag += _T("_");
        strTag += item.strName;
        strTag += _T("_");
        strTag += item.strKeyToken;
        strTag += _T("_");
        strTag += item.strVersion;

        strSxSFolder = _T("C:\\Windows\\winsxs\\");
        strSxSFile = _T("C:\\Windows\\winsxs\\Manifests\\");

        strDest = strExportPath;
        if(!ExportFolder(strTag, item.strName, strSxSFolder, strDest, strMsg)
            || !ExportFile(strTag, item.strName, strSxSFile, strDest, strMsg))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CSxSParser::ExportFolder(LPCTSTR szTag, LPCTSTR szName, LPCTSTR szSrc, LPCTSTR szDst, CString& strMsg) const
{
    CString strTag(szTag);
    strTag += _T("*");

    CString strSrc(szSrc);
    if(strSrc[strSrc.GetLength() - 1] != _T('\\'))
        strSrc += _T('\\');

    CString strDst(szDst);
    if(strDst[strDst.GetLength() - 1] != _T('\\'))
        strDst += _T('\\');

    StringList folders = FindMatchedFSObjects(strSrc, strTag, FALSE);

    UINT uCount = folders.GetSize();
    if(uCount == 0)
    {
        strMsg.Format(IDS_ERR_SXS_FOLDER_NOT_EXISTS, szTag);
        return FALSE;
    }

    CString strTo(strDst);
    strTo.AppendChar(0);
    CString orgFolder, targetFolder;

    for(UINT i=0; i<uCount; ++ i)
    {
        CString strFrom(strSrc + folders[i]);
        strFrom.AppendChar(0);

        if(!MakeEmptyFolder(strDst + szName))
        {
            strMsg.LoadString(IDS_ERR_MAKE_EMPTY_FOLDER);
            return FALSE;
        }

        SHFILEOPSTRUCT fo = {0};
        fo.hwnd = NULL;
        fo.wFunc = FO_COPY;
        fo.pFrom = strFrom;
        fo.pTo = strTo;
        fo.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_NORECURSION;
        if(0 != SHFileOperation(&fo))
        {
            strMsg.LoadString(IDS_ERR_COPY_FOLDERS);
            return FALSE;
        }

        orgFolder = strDst;
        orgFolder += folders[i];

        targetFolder = strDst;
        targetFolder += szName;

        if(!::MoveFile(orgFolder, targetFolder))
        {
            strMsg.LoadString(IDS_ERR_COPY_FOLDERS);
            return FALSE;
        }
    }
    return TRUE;
}

BOOL CSxSParser::ExportFile(LPCTSTR szTag, LPCTSTR szName, LPCTSTR szSrc, LPCTSTR szDst, CString& strMsg) const
{
    CString strTag(szTag);
    strTag += _T("*.manifest");

    CString strSrc(szSrc);
    if(strSrc[strSrc.GetLength() - 1] != _T('\\'))
        strSrc += _T('\\');

    StringList folders = FindMatchedFSObjects(strSrc, strTag, TRUE);

    UINT uCount = folders.GetSize();
    if(uCount == 0)
    {
        strMsg.Format(IDS_ERR_SXS_FOLDER_NOT_EXISTS, szTag);
        return FALSE;
    }

    CString strDst(szDst);
    if(strDst[strDst.GetLength() - 1] != _T('\\'))
        strDst += _T('\\');
    strDst += szName;

    for(UINT i=0; i<uCount; ++ i)
    {
        CString strFrom(strSrc + folders[i]);
        CString strTo(strDst);
        strTo += _T("\\");
        strTo += szName;
        strTo += _T(".manifest");

        if(!::CopyFile(strFrom, strTo, FALSE))
        {
            strMsg.LoadString(IDS_ERR_COPY_FILES);
            return FALSE;
        }
    }
    return TRUE;
}

StringList CSxSParser::FindMatchedFSObjects(LPCTSTR szParentFolder, LPCTSTR szFilter, BOOL bIsFile) const
{
    CString strParent(szParentFolder);
    if(strParent[strParent.GetLength() - 1] != _T('\\'))
        strParent += _T('\\');
    CString strFilter(strParent);
    strFilter += szFilter;

    StringList listResult;
    WIN32_FIND_DATA data;
    HANDLE hFind = ::FindFirstFile(strFilter, &data);
    if(hFind == INVALID_HANDLE_VALUE)
        return listResult;

    CString strFilePath;
    do 
    {
        if(_tcscmp(data.cFileName, _T(".")) == 0
            || _tcscmp(data.cFileName, _T("..")) == 0)
            continue;

        if(bIsFile && ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if(!bIsFile && ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
            continue;

        strFilePath = data.cFileName;
        listResult.Add(strFilePath);

    } while (::FindNextFile(hFind, &data));
    ::FindClose(hFind);

    return listResult;
}

BOOL CSxSParser::MakeEmptyFolder(LPCTSTR szFolder) const
{
    DWORD dwAttr = ::GetFileAttributes(szFolder);
    if(dwAttr == INVALID_FILE_ATTRIBUTES)
        return TRUE;

    CString strFrom(szFolder);
    strFrom.AppendChar(0);

    SHFILEOPSTRUCT fo = {0};
    fo.hwnd = NULL;
    fo.wFunc = FO_DELETE;
    fo.pFrom = strFrom;
    fo.pTo = NULL;
    fo.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_NORECURSION;
    return (0 == SHFileOperation(&fo));
}

BOOL CSxSParser::AddAssemblyItem(const CSxSItem& item)
{
    int nItem = m_SxSList.Find(item);
    if(nItem == -1)
        return m_SxSList.Add(item);

    CSxSItem& existedItem = m_SxSList[nItem];
    UINT nSize = item.listFiles.GetSize();
    for(UINT i=0; i<nSize; ++ i)
    {
        if(existedItem.listFiles.Find(item.listFiles[i]) == -1)
            existedItem.listFiles.Add(item.listFiles[i]);
    }
    return TRUE;
}

BOOL CSxSParser::IsIgnoreItem(const CSxSItem& item) const
{
    return (item.strName == _T("Microsoft.Windows.Common-Controls")
        && item.strVersion[0] - _T('0') <= 6);
}

