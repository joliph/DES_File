
// DES_FileDlg.h: 头文件
//

#pragma once


// CDESFileDlg 对话框
class CDESFileDlg : public CDialogEx
{
// 构造
public:
	CDESFileDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DES_FILE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	BOOL Check();
	void CBC_Xor(bool *Bit_Mingwen,char *now,char *last);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString FilePath;
	CString CString_Key;
	HANDLE	File_Handle;
	DWORD	FileSize;
	char		*pFileBuf;
	char		*pNewFileBuf;
	BOOL		Ls_CBC;
	afx_msg void OnBnClickedChose();
	afx_msg void OnBnClickedEncrypt();
	afx_msg void OnBnClickedDecrypt();
	double Time_Used;
};
