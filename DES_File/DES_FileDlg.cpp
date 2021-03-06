
// DES_FileDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "DES_File.h"
#include "DES_FileDlg.h"
#include "afxdialogex.h"
#include "DES.h"
#include <ctime>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDESFileDlg 对话框



CDESFileDlg::CDESFileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DES_FILE_DIALOG, pParent)
	, FilePath(_T(""))
	, CString_Key(_T(""))
	, Ls_CBC(FALSE)
	, Time_Used(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDESFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FILE, FilePath);
	DDX_Text(pDX, IDC_KEY, CString_Key);
	DDX_Radio(pDX, IDC_ECB, Ls_CBC);
	DDX_Text(pDX, IDC_TIME, Time_Used);
}

BEGIN_MESSAGE_MAP(CDESFileDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOSE, &CDESFileDlg::OnBnClickedChose)
	ON_BN_CLICKED(IDC_ENCRYPT, &CDESFileDlg::OnBnClickedEncrypt)
	ON_BN_CLICKED(IDC_DECRYPT, &CDESFileDlg::OnBnClickedDecrypt)
//	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CDESFileDlg 消息处理程序

BOOL CDESFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CDESFileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDESFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CDESFileDlg::OnBnClickedChose(){
	CFileDialog dlg(TRUE, NULL, NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("All Files (*.*)|*.*||"), NULL);
	if (dlg.DoModal() == IDOK)
		FilePath = dlg.GetPathName();
	else
		return;
	UpdateData(FALSE);
	File_Handle = CreateFile(FilePath,
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (File_Handle == INVALID_HANDLE_VALUE) {
		MessageBoxA("加载文件失败！","Error", MB_OK);
		File_Handle = NULL;
		return;
	}
	FileSize = GetFileSize(File_Handle, NULL);
	int need_add = 0;
	if(FileSize % 8 != 0){
		need_add = 8 - (FileSize % 8);
	}
	pFileBuf = new char[FileSize+ need_add];
	DWORD ReadSize = 0;
	ReadFile(File_Handle, pFileBuf, FileSize, &ReadSize, NULL);
	memset(pFileBuf + FileSize, 0, need_add * sizeof(char));
	FileSize += need_add;
	CloseHandle(File_Handle);
	File_Handle = NULL;
}


void CDESFileDlg::OnBnClickedEncrypt(){
	UpdateData(TRUE);
	if (!Check()) {
		return;
	}
	char *Char_Key= (char*)CString_Key.GetBuffer(0);
	DES_Key Input_Key(Char_Key);
	int count = FileSize / 8;
	pNewFileBuf = new char[FileSize];
	clock_t startTime = clock();    //开始时刻
	for(int i=0;i<count;i++){
		bool Bit_Mingwen[64];
		CharToBit(Bit_Mingwen, pFileBuf);
		if (Ls_CBC && i != 0) {
			CBC_Xor(Bit_Mingwen,pFileBuf, pNewFileBuf-8);
		}
		Des_Run(Bit_Mingwen, Input_Key, pNewFileBuf);
		pFileBuf += 8;
		pNewFileBuf += 8;
	}
	clock_t endTime = clock();      //结束时刻
	Time_Used=double(endTime - startTime) / CLOCKS_PER_SEC;
	UpdateData(FALSE);
	pNewFileBuf -= count * 8;
	int n=FilePath.ReverseFind('.');
	CString NewFilePath = FilePath.Left(n+1);
	NewFilePath+="encrypted";
	HANDLE NewFile_Handle=CreateFile(NewFilePath,
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD WriteSize = 0;
	WriteFile(NewFile_Handle, pNewFileBuf, FileSize, &WriteSize, NULL);
	CloseHandle(NewFile_Handle);
	NewFile_Handle = NULL;
	MessageBoxA("加密完成", "Nice", NULL);
	delete Char_Key;
	delete pNewFileBuf;
	Time_Used = 0;
}

void CDESFileDlg::OnBnClickedDecrypt(){
	UpdateData(TRUE);
	if (!Check()) {
		return;
	}
	char *Char_Key = (char*)CString_Key.GetBuffer(0);
	DES_Key Input_Key(Char_Key);
	int count = FileSize / 8;
	pNewFileBuf = new char[FileSize];
	for (int i = 0; i<count; i++) {
		bool Bit_Mingwen[64];
		CharToBit(Bit_Mingwen, pFileBuf);
		Re_Des_Run(Bit_Mingwen, Input_Key, pNewFileBuf);
		if (Ls_CBC && i != 0) {
			CBC_Xor(Bit_Mingwen, pNewFileBuf, pFileBuf-8);
			ShowResult(Bit_Mingwen, pNewFileBuf);
		}
		pFileBuf += 8;
		pNewFileBuf += 8;
	}
	pNewFileBuf -= count * 8;
	int n = FilePath.ReverseFind('.');
	CString NewFilePath = FilePath.Left(n + 1);
	NewFilePath += "decrypted";
	HANDLE NewFile_Handle = CreateFile(NewFilePath,
		GENERIC_READ | GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD WriteSize = 0;
	WriteFile(NewFile_Handle, pNewFileBuf, FileSize, &WriteSize, NULL);
	CloseHandle(NewFile_Handle);
	NewFile_Handle = NULL;
	MessageBoxA("解密完成", "Nice", NULL);
	delete Char_Key;
	delete pNewFileBuf;
}


BOOL CDESFileDlg::Check() {
	//检测更正
	if (CString_Key.GetLength() != 8) {
		MessageBoxA("密钥应为8位","Error",MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

void CDESFileDlg::CBC_Xor(bool *Bit_Mingwen, char *now, char *last) {
	bool temp1[64];
	bool temp2[64];
	CharToBit(temp1, now);
	CharToBit(temp2, last);
	Xor(temp1,temp2, 64);
	memcpy(Bit_Mingwen, temp1, 64);
}
