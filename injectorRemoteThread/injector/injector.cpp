// injector.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "injector.h"
#include <windows.h>
#include "resource.h"
#include <TlHelp32.h>
#include <stdio.h>

INT_PTR CALLBACK Dlgproc(HWND Arg1,UINT Arg2,WPARAM Arg3,LPARAM Arg4);




/*
	进程Pid取进程句柄
*/
HANDLE ProcessPid2Handle(DWORD pid) {
	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

/*
	进程名称获取进程Pid
*/
DWORD ProcessName2Pid(CHAR * ProcessName) {
	// 获取系统进程快照
	HANDLE processAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	tagPROCESSENTRY32 processEntry = {};
	processEntry.dwSize = sizeof(tagPROCESSENTRY32);
	DWORD processId = 0;
	do {
		if (strcmp(ProcessName, processEntry.szExeFile) == 0) {
			// 获取到PID
			processId = processEntry.th32ProcessID;
			break;
		}
		OutputDebugString(processEntry.szExeFile);
	} while (Process32Next(processAll, &processEntry));
	return processId;
}

/*
	注入
*/
VOID injectDll() {

	CHAR dllPath[0x100]  = "TEST.dll";
	CHAR processName[] = "TEST.exe";


	DWORD pid = ProcessName2Pid((CHAR*)processName);
	if (pid == 0) {
		MessageBoxA(NULL, "找不到进程!", "Error", 0);
		return;
	}

	HANDLE hPro = ProcessPid2Handle(pid);
	if (hPro == NULL) {
		MessageBoxA(NULL, "打开进程失败!", "Error", 0);
		return;
	}

	// 申请一块内存
	LPVOID  dllBaseAdd = VirtualAllocEx(hPro, NULL, strlen(dllPath), MEM_COMMIT, PAGE_READWRITE);
	if(dllBaseAdd == NULL){
		MessageBoxA(NULL, "申请内存失败!", "Error", 0);
		return;
	}

	// 写内存
	if (WriteProcessMemory(hPro, dllBaseAdd,dllPath,strlen(dllPath),NULL) == 0) {
		MessageBoxA(NULL, "写入内存失败!", "Error", 0);
		return;
	}

	char out[100];
	sprintf_s(out, "\n写入地址为:%p", dllBaseAdd);
	OutputDebugString(out);

	// 启动远程线程
	HMODULE kernel32 = GetModuleHandle("Kernel32.dll");
	LPVOID loadLib = GetProcAddress(kernel32, "LoadLibraryA");
	
	if (CreateRemoteThread(hPro, NULL, 0, (LPTHREAD_START_ROUTINE)loadLib, dllBaseAdd, 0, NULL) == NULL) {
		MessageBoxA(NULL, "线程启动失败!", "Error", 0);
		return;
	}
}







int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)

{
	DialogBoxA(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,&Dlgproc);
	return 0;
}

/*
	回调函数，处理消息
	Arg1:对话框句柄
	Arg2:消息
	Arg3:特定信息
	Arg4:特定信息
*/
INT_PTR CALLBACK Dlgproc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG) {
		// 首次加载Dialog
		// MessageBoxA(NULL, "Hello", "info", 0);
	}
	else if (uMsg == WM_CLOSE){
		EndDialog(hwndDlg, 0);
	}
	else if (uMsg == WM_COMMAND) {
		// 控件消息
		switch (wParam)
		{
		case ID_INJECT:
			injectDll();
			break;
		case ID_UNIJECET:
			// MessageBoxA(NULL, "卸载成功", "info", 0);
			break;
		default:
			break;
		}

	}
	return false;
}

