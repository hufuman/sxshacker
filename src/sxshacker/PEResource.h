#pragma once



/**
 * class to help get manifest of exe/dll
 */
class CPEResource
{
public:
    CPEResource();
    ~CPEResource(void);

public:
    BOOL Open(CString strPEPath);

    BOOL GetManifest(LPVOID& pManifest, DWORD& dwLength);

protected:
    void Close();

protected:
    CString     m_strPEPath;

    HMODULE     m_hModule;
    HRSRC       m_hManifest;
};
