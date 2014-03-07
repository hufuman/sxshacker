// maindlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////


#pragma once


#include "AboutDlg.h"
#include "Util.h"
#include "SxSParser.h"
#include "WndLayout.h"


class CMainDlg : public CDialogImpl<CMainDlg>, public CMessageFilter
{
public:
	enum { IDD = IDD_DIALOG_MAIN };


	CMainDlg()
	{
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return ::IsDialogMessage(m_hWnd, pMsg);
	}

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

        COMMAND_ID_HANDLER(IDC_BTN_REFRESH, OnBtnRefresh)
        COMMAND_ID_HANDLER(IDC_BTN_BROWSE, OnBtnBrowse)
        COMMAND_ID_HANDLER(IDC_BTN_EXPORT, OnBtnExport)

		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)

	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

		// Add "About..." menu item to system menu.

		// IDM_ABOUTBOX must be in the system command range.
		_ASSERTE((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
		_ASSERTE(IDM_ABOUTBOX < 0xF000);

		CMenu SysMenu = GetSystemMenu(FALSE);
		if(::IsMenu(SysMenu))
		{
			TCHAR szAboutMenu[256];
			if(::LoadString(_Module.GetResourceInstance(), IDS_ABOUTBOX, szAboutMenu, 255) > 0)
			{
				SysMenu.AppendMenu(MF_SEPARATOR);
				SysMenu.AppendMenu(MF_STRING, IDM_ABOUTBOX, szAboutMenu);
			}
		}
		SysMenu.Detach();

		// register object for message filtering
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);

        m_strAppName.LoadString(IDS_APP_NAME);

        // 
        Util::EnableDrop(m_hWnd);

        m_Tree.Attach(GetDlgItem(IDC_TREE_DEPENDENCIES));

        InitLayout();

		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
        return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

    LRESULT OnBtnRefresh(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        // whether tree are generated already
        if(m_Tree.GetCount() == 0)
            return 0;

        m_Tree.DeleteAllItems();
        BOOL bResult = m_SxsParser.Reparse();
        GenerateTree();
        if(!bResult)
        {
            CString strMsg;
            strMsg.LoadString(IDS_ERR_REPARSE);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);
        }

        return 0;
    }

    LRESULT OnBtnBrowse(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strResult = Util::BrowserForFolder(m_hWnd);

        if(!strResult.IsEmpty())
        {
            EnableStep3(TRUE);
            GetDlgItem(IDC_EDIT_EXPORT_PATH).SetWindowText(strResult);
        }

        return 0;
    }

    LRESULT OnBtnExport(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        CString strExportPath;
        GetDlgItem(IDC_EDIT_EXPORT_PATH).GetWindowText(strExportPath);

        if(strExportPath.IsEmpty())
        {
            CString strMsg;
            strMsg.LoadString(IDS_ERR_EMPTY_EXPORT_PATH);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);
            return 0;
        }

        DWORD dwAttr = ::GetFileAttributes(strExportPath);
        if(INVALID_FILE_ATTRIBUTES == dwAttr || ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY))
        {
            CString strMsg;
            strMsg.LoadString(IDS_ERR_NOT_DIRECTORY);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);
            return 0;
        }

        CString strMsg;
        if(m_SxsParser.Export(strExportPath, strMsg))
        {
            strMsg.LoadString(IDS_OK_EXPORT);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            strMsg.LoadString(IDS_ERR_EXPORT);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);
            return 0;
        }
        return 0;
    }

	void CloseDialog(int nVal)
	{
		DestroyWindow();
		::PostQuitMessage(nVal);
	}

	LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		UINT uCmdType = (UINT)wParam;

		if((uCmdType & 0xFFF0) == IDM_ABOUTBOX)
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}
		else
			bHandled = FALSE;

		return 0;
	}

    LRESULT OnDropFiles(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = TRUE;
        HDROP hDrop = reinterpret_cast<HDROP>(wParam);

        TCHAR szPath[MAX_PATH * 2] = {0};
        UINT uCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, szPath, _countof(szPath));
        if(uCount == 0)
        {
            CString strMsg;
            strMsg.LoadString(IDS_NO_FILE_DROPPED);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);
            return 0;
        }

        EnableStep2(FALSE);
        EnableStep3(FALSE);

        BOOL bResult = TRUE;
        m_SxsParser.Clear();
        CString strLastPath;
        for(UINT i=0; i<uCount; ++ i)
        {
            if(::DragQueryFile(hDrop, i, szPath, _countof(szPath)) > 0)
            {
                if(!m_SxsParser.AddFile(szPath))
                {
                    bResult = FALSE;
                }
                else
                {
                    strLastPath = szPath;
                }
            }
        }

        if(!bResult)
        {
            CString strMsg;
            strMsg.LoadString(IDS_ERR_SXS_PARSE);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONERROR);

            if(m_SxsParser.GetSxSList().GetSize() == 0)
                return 0;
        }

        // check if no dependencies needed
        if(m_SxsParser.GetSxSList().GetSize() == 0)
        {
            CString strMsg;
            strMsg.LoadString(IDS_MSG_NO_DEPENDENCIES);
            MessageBox(strMsg, m_strAppName, MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        GenerateTree();

        // Enable Next Step
        EnableStep2(TRUE);

        CWindow wndExportPath = GetDlgItem(IDC_EDIT_EXPORT_PATH);
        if(wndExportPath.GetWindowTextLength() == 0)
        {
            int pos = strLastPath.ReverseFind(_T('\\'));
            if(pos != -1)
            {
                strLastPath = strLastPath.Mid(0, pos);
                wndExportPath.SetWindowText(strLastPath);
                EnableStep3(TRUE);
            }
        }

        return 0;
    }

    void EnableStep2(BOOL bEnable)
    {
        m_Tree.EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_BROWSE).EnableWindow(bEnable);
        // GetDlgItem(IDC_EDIT_EXPORT_PATH).EnableWindow(bEnable);

        if(bEnable && GetDlgItem(IDC_EDIT_EXPORT_PATH).GetWindowTextLength() > 0)
            EnableStep3(TRUE);
    }

    void EnableStep3(BOOL bEnable)
    {
        GetDlgItem(IDC_BTN_EXPORT).EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_REFRESH).EnableWindow(bEnable);
    }

    void GenerateTree()
    {
        m_Tree.DeleteAllItems();

        CString strTemp;
        const SxSList& list = m_SxsParser.GetSxSList();
        UINT uCount = list.GetSize();
        for(UINT i=0; i<uCount; ++ i)
        {
            const CSxSItem& item = list[i];

            // Item
            strTemp = item.strName;
            if(item.bIsIgnoreItem)
            {
                strTemp += _T(" -- (Ignored)");
            }

            HTREEITEM hItem = m_Tree.InsertItem(strTemp, NULL, NULL);

            // Type
            strTemp = _T("Type: ") + item.strType;
            m_Tree.InsertItem(strTemp, hItem, NULL);

            // Version
            strTemp = _T("Version: ") + item.strVersion;
            m_Tree.InsertItem(strTemp, hItem, NULL);

            // KeyToken
            strTemp = _T("KeyToken: ") + item.strKeyToken;
            m_Tree.InsertItem(strTemp, hItem, NULL);

            // ProcessArch
            strTemp = _T("ProcessArch: ") + item.strProcessor;
            m_Tree.InsertItem(strTemp, hItem, NULL);

            // Files
            HTREEITEM hFile = m_Tree.InsertItem(_T("Files"), hItem, NULL);
            UINT uSize = item.listFiles.GetSize();
            for(UINT i=0; i<uSize; ++ i)
            {
                m_Tree.InsertItem(item.listFiles[i], hFile, NULL);
            }
        }
    }

    // Layout
    void InitLayout()
    {
        m_WndLayout.Init(m_hWnd);
        m_WndLayout.AddControlById(IDC_LABEL_STEP1, Layout_Top | Layout_Left);
        m_WndLayout.AddControlById(IDC_LABEL_STEP2, Layout_Top | Layout_Left);
        m_WndLayout.AddControlById(IDC_TREE_DEPENDENCIES, Layout_VFill | Layout_HFill);
        m_WndLayout.AddControlById(IDC_LABEL_STEP3, Layout_Bottom | Layout_Left);
        m_WndLayout.AddControlById(IDC_EDIT_EXPORT_PATH, Layout_Bottom | Layout_HFill);
        m_WndLayout.AddControlById(IDC_BTN_BROWSE, Layout_Bottom | Layout_Right);
        m_WndLayout.AddControlById(IDC_BTN_EXPORT, Layout_Bottom | Layout_Right);
        m_WndLayout.AddControlById(IDC_BTN_REFRESH, Layout_Top | Layout_Right);
    }

private:
    CTreeViewCtrl   m_Tree;
    CSxSParser      m_SxsParser;
    CWndLayout      m_WndLayout;
    CString         m_strAppName;
};
