/* x86.Virtualizer
 * Copyright 2007 ReWolf
 * Contact:
 * rewolf@rewolf.pl
 * http://rewolf.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <windows.h>
#include <cstdio>
#include <ctime>
#include "protect.h"

#include "resource.h"

#define MAKE_VM_CALL(imgBase, funcAddr, funcRVA, vmedFuncAddr, fSize, vmStartRVA) \
	*(BYTE*)(funcAddr) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 1) = (vmedFuncAddr); \
	*((BYTE*)(funcAddr) + 5) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 6) = imgBase + (funcRVA) + 16; \
	*((BYTE*)(funcAddr) + 10) = 0x68; \
	*(DWORD*)((BYTE*)(funcAddr) + 11) = imgBase + (vmStartRVA); \
	*((BYTE*)(funcAddr) + 15) = 0xC3; \
	memset((BYTE*)(funcAddr) + 16, 0x90, (fSize) - 16);

#define MAKE_VM_CALL2(imgBase, funcAddr, funcRVA, vmedFuncRVA, fSize, vmStartRVA, inLdrAddr, inLdrRVA) \
	*(BYTE*)(funcAddr) = 0xE9; \
	*(DWORD*)((BYTE*)(funcAddr) + 1) = (inLdrRVA) - (funcRVA) - 5; \
	memset((BYTE*)(funcAddr) + 5, 0x90, (fSize) - 5); \
	*(BYTE*)(inLdrAddr) = 0xE8; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 1) = 0; \
	*((BYTE*)(inLdrAddr) + 5) = 0x9C; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 6) = 0x04246C81; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 10) = (inLdrRVA) - (vmedFuncRVA) + 5; \
	*((BYTE*)(inLdrAddr) + 14) = 0x9D; \
	*((BYTE*)(inLdrAddr) + 15) = 0xE8; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 16) = 0; \
	*((BYTE*)(inLdrAddr) + 20) = 0x9C; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 21) = 0x04246C81; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 25) = (inLdrRVA) - ((funcRVA) + 5) + 20; \
	*((BYTE*)(inLdrAddr) + 29) = 0x9D; \
	*(BYTE*)((BYTE*)(inLdrAddr) + 30) = 0xE9; \
	*(DWORD*)((BYTE*)(inLdrAddr) + 31) = (vmStartRVA) - ((inLdrRVA) + 30) - 5;

#define VM_CALL_SIZE 35

int vm_protect_vm(BYTE* vm_in_exe, BYTE* outBuf, DWORD imgBase, DWORD vmRVA, DWORD newRVA)
{
	BYTE* hVMMemory;
	DWORD vmInit;
	DWORD vmStart;
	int vmSize;
	DWORD* hVMImg;
	if (!outBuf) 
	{
		vmSize = vm_init(&hVMMemory, &vmInit, &vmStart);
		hVMImg = (DWORD*)vm_getVMImg();
	}
	else 
	{
		hVMImg = (DWORD*)vm_getVMImg();
		vmSize = vm_getVMSize();
		DWORD _ssss = (*(DWORD*)(hVMImg + 7))*4 + (*(DWORD*)(hVMImg + 8))*8 + 4;
		vmInit = *(DWORD*)(hVMImg + 2) - _ssss;
		vmStart = *(DWORD*)(hVMImg + 3) - _ssss;
		hVMMemory = (BYTE*)hVMImg + _ssss;
	}

	int cnt = hVMImg[7];
	cnt = hVMImg[cnt];
	int curOutPos = 0;

	if (outBuf) memmove(outBuf, hVMMemory, vmSize);
	curOutPos += vmSize;	

	for (int i = 0; i < cnt; i++)
	{
		int cur = hVMImg[7] + 2*i + 1;
		if (!outBuf)
		{
			hVMImg[cur] -= (0x401000 + hVMImg[7]*4 + cnt*8 + 4);
			hVMImg[cur + 1] -= (0x401000 + hVMImg[7]*4 + cnt*8 + 4);
		}

		int lalala = curOutPos;
		BYTE* __outBuf;
		if (outBuf) __outBuf = outBuf + curOutPos;
		else __outBuf = 0;
		curOutPos += vm_protect(vm_in_exe + hVMImg[cur], hVMImg[cur + 1] - hVMImg[cur], __outBuf, vmRVA + hVMImg[cur], 0, imgBase);
		//MAKE_VM_CALL(imgBase, vm_in_exe + hVMImg[cur], vmRVA + hVMImg[cur], imgBase + newRVA + lalala, hVMImg[cur + 1] - hVMImg[cur], newRVA + vmStart);
		if (outBuf)
		{
			MAKE_VM_CALL2(imgBase, vm_in_exe + hVMImg[cur], vmRVA + hVMImg[cur], newRVA + lalala, hVMImg[cur + 1] - hVMImg[cur], newRVA + vmStart, outBuf + curOutPos, newRVA + curOutPos);
		}
		curOutPos += VM_CALL_SIZE;
	}

	return curOutPos;
}
//-----------------------------------------------------------------------------
DWORD ddFrom;
DWORD ddTo;
int WINAPI AddDialogProc(HWND hDlg, UINT uMSg, WPARAM wParam, LPARAM lParam)
{
	switch (uMSg)
	{
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			{
				switch (wParam & 0xFFFF)
				{
					case BTN_ADD_NO:						
						EndDialog(hDlg, 0);
						break;
					case BTN_ADD_YES:
						char temp[20];
						ddFrom = 0;
						ddTo = 0;
						GetDlgItemText(hDlg, EDT_FROM, temp, 20);
						sscanf(temp, "%x", &ddFrom);
						GetDlgItemText(hDlg, EDT_TO, temp, 20);
						sscanf(temp, "%x", &ddTo);
						if (ddTo && ddFrom) EndDialog(hDlg, 1);
						else MessageBox(hDlg, "Error", "Error", MB_ICONERROR);
						break;
				}
			}
			break;
	}
	return 0;
}

DWORD rva2raw(WORD NumOfSections, IMAGE_SECTION_HEADER* FSH, DWORD rva)
{
	for (int i = NumOfSections-1; i >= 0; i--)
		if (FSH[i].VirtualAddress <= rva) 
			return FSH[i].PointerToRawData + rva - FSH[i].VirtualAddress;
	return 0xFFFFFFFF;
}

DWORD searchFunction(BYTE* exeMem, char* functionName)
{
	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)(exeMem + ((IMAGE_DOS_HEADER*)exeMem)->e_lfanew);
	IMAGE_SECTION_HEADER* ish = (IMAGE_SECTION_HEADER*)(exeMem + ((IMAGE_DOS_HEADER*)exeMem)->e_lfanew + inh->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	IMAGE_IMPORT_DESCRIPTOR* imports = (IMAGE_IMPORT_DESCRIPTOR*)inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	imports = (IMAGE_IMPORT_DESCRIPTOR*)(exeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, (DWORD)imports));
	bool found = false;
	while (!found && imports->Name)
	{
		char* libName = (char*)exeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, imports->Name);
		if (!stricmp(libName, "kernel32.dll"))
		{
			found = true;
			DWORD* thunks = (DWORD*)(imports->FirstThunk);
			thunks = (DWORD*)(exeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, imports->FirstThunk));
			bool found2 = false;
			int k = 0;
			while (!found2 && *thunks)
			{
				char* curName = (char*)exeMem + rva2raw(inh->FileHeader.NumberOfSections, ish, *thunks);
				if (!stricmp(curName + 2, functionName)) 
				{
					return imports->FirstThunk + k*4;
					found2 = true;
				}
				k++;
				thunks++;
			}
		}
		imports++;
	}

}

#define ERROR(a) { MessageBox(0, a, "Error", MB_ICONERROR); return; }
#define ERROR2(a) \
	{ \
		MessageBox(0, a, "Error", MB_ICONERROR); \
		GlobalFree(items); \
		GlobalFree(items2); \
		vm_free(); \
		return; \
	}

#define TRUNC(a, b) (a + (b - ((a % b) ? (a % b) : b)))

void doProtect(HWND listBox, bool vmovervm, char* fileName)
{
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) ERROR("Cannot open input file.");
	DWORD tmp;
	DWORD fSize = GetFileSize(hFile, 0);
	BYTE* hInMem = (BYTE*)GlobalAlloc(GMEM_FIXED, fSize);
	if (!hInMem) ERROR("Cannot allocate memory.");
	ReadFile(hFile, hInMem, fSize, &tmp, 0);
	CloseHandle(hFile);

	IMAGE_NT_HEADERS* inh = (IMAGE_NT_HEADERS*)(hInMem + ((IMAGE_DOS_HEADER*)hInMem)->e_lfanew);
	IMAGE_SECTION_HEADER* ish = (IMAGE_SECTION_HEADER*)(hInMem + ((IMAGE_DOS_HEADER*)hInMem)->e_lfanew + inh->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER) + 4);

	//relocs check
	DWORD rel = 0;
	if (inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
	{
		rel = rva2raw(inh->FileHeader.NumberOfSections, ish, inh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		if (rel == 0xFFFFFFFF) ERROR("Invalid relocations RVA.");
		rel += (DWORD)hInMem;
	}

	//build protection table:
	int itemsCnt = SendMessage(listBox, LB_GETCOUNT, 0, 0);
	if (!itemsCnt) ERROR("Nothing to protect (add at least one range).");
	DWORD* items = (DWORD*)GlobalAlloc(GMEM_FIXED, itemsCnt*8);
	DWORD* items2 = (DWORD*)GlobalAlloc(GMEM_FIXED, itemsCnt*8);
	//vm init
	BYTE* hVMMemory;
	DWORD vmInit;
	DWORD vmStart;
	srand(time(0));
	int vmSize = vm_init(&hVMMemory, &vmInit, &vmStart);
	//
	int protSize = 0;
	for (int i = 0; i < itemsCnt; i++)
	{
		char temp[25];
		SendMessage(listBox, LB_GETTEXT, i, (LPARAM)temp);
		temp[8] = 0;
		sscanf(temp, "%x", &items[i*2]);
		sscanf(temp + 11, "%x", &items[i*2 + 1]);
		items[i*2] -= inh->OptionalHeader.ImageBase;
		items[i*2 + 1] -= inh->OptionalHeader.ImageBase;

		items2[i*2] = rva2raw(inh->FileHeader.NumberOfSections, ish, items[i*2]);
		if (items2[i*2] == 0xFFFFFFFF) ERROR2("Invalid range start");
		items2[i*2 + 1] = items[i*2 + 1] - items[i*2];		//size
		int t = vm_protect(hInMem + items2[i*2], items2[i*2 + 1], 0, items[i*2], (BYTE*)rel, inh->OptionalHeader.ImageBase);
		if (t == -1) ERROR2("[SIZE] Protection failed.");
		protSize += t;
	}
	//loader size = 0x3C

	//first protection layer
	//get new section rva
	DWORD newRVA = (ish + inh->FileHeader.NumberOfSections - 1)->VirtualAddress + TRUNC((ish + inh->FileHeader.NumberOfSections - 1)->Misc.VirtualSize, inh->OptionalHeader.SectionAlignment);	
	DWORD vAlloc = searchFunction(hInMem, "VirtualAlloc");
	//DWORD newSecSize = TRUNC((vmSize + protSize + 0x3C + itemsCnt*0x1F), inh->OptionalHeader.FileAlignment);
	DWORD newSecSize = vmSize + protSize + 0x3C + itemsCnt*VM_CALL_SIZE;
	BYTE* hNewMem = (BYTE*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, newSecSize);
	int curPos = 0;
	memmove(hNewMem, hVMMemory, vmSize);
	curPos += vmSize;

	//setting new entry point
	DWORD oldEntry = inh->OptionalHeader.AddressOfEntryPoint;
	inh->OptionalHeader.AddressOfEntryPoint = newRVA + vmSize;

	//VirtualAlloc at 0xA0F8
	static char loaderAlloc[] = 
	{
		0xE8, 0x00, 0x00, 0x00, 0x00,       //CALL    vm_test2.004120DA
		0x5B,                               //POP     EBX
		0x81, 0xEB, 0xDA, 0x20, 0x01, 0x00, //SUB     EBX,120DA
		0x6A, 0x40,							//PUSH    40
		0x68, 0x00, 0x10, 0x00, 0x00,       //PUSH    1000
		0x68, 0x00, 0x10, 0x00, 0x00,       //PUSH    1000
		0x6A, 0x00,                         //PUSH    0
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0x10,                         //CALL    [EAX]
		0x53,                               //PUSH    EBX
		0x05, 0x00, 0x10, 0x00, 0x00,       //ADD     EAX,1000
		0x50,                               //PUSH    EAX
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0xD0,                         //CALL    EAX
		0xB8, 0x34, 0x12, 0x00, 0x00,       //MOV     EAX,1234
		0x03, 0xC3,                         //ADD     EAX,EBX
		0xFF, 0xE0                          //JMP     EAX
	};
	//correcting loader:
	*(DWORD*)(loaderAlloc + 8) = newRVA + vmSize + 5;
	*(DWORD*)(loaderAlloc + 27) = vAlloc;
	*(DWORD*)(loaderAlloc + 43) = newRVA + vmInit;
	*(DWORD*)(loaderAlloc + 52) = oldEntry;
	memmove(hNewMem + vmSize, loaderAlloc, sizeof(loaderAlloc));
	curPos += sizeof(loaderAlloc);

	for (int i = 0; i < itemsCnt; i++)
	{
		int _tts = vm_protect(hInMem + items2[i*2], items2[i*2 + 1], hNewMem + curPos, items[i*2], (BYTE*)rel, inh->OptionalHeader.ImageBase);
		MAKE_VM_CALL2(inh->OptionalHeader.ImageBase, hInMem + items2[i*2], items[i*2], newRVA + curPos, items2[i*2 + 1], newRVA + vmStart, hNewMem + curPos + _tts, newRVA + curPos + _tts);
		curPos += _tts + VM_CALL_SIZE;
	}

	if (vmovervm)
	{
		//chuj wi czemu ;p
		//newSecSize -= 5;
		//
		int vmovmSize = vm_protect_vm(hNewMem, 0, inh->OptionalHeader.ImageBase, newRVA, newRVA + curPos);
		//BYTE* tmpMemPtr = (BYTE*)GlobalReAlloc(hNewMem, newSecSize + vmovmSize, GMEM_FIXED | GMEM_ZEROINIT);		
		BYTE* tmpMemPtr = (BYTE*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, newSecSize + vmovmSize + sizeof(loaderAlloc));
		memmove(tmpMemPtr, hNewMem, newSecSize);
		GlobalFree(hNewMem);
		hNewMem = tmpMemPtr;
		vm_protect_vm(hNewMem, hNewMem + newSecSize, inh->OptionalHeader.ImageBase, newRVA, newRVA + curPos);
		curPos += vmovmSize;
		
		oldEntry = inh->OptionalHeader.AddressOfEntryPoint;
		inh->OptionalHeader.AddressOfEntryPoint = newRVA + curPos;

		*(DWORD*)(loaderAlloc + 8) = newRVA + curPos + 5;
		//*(DWORD*)(loaderAlloc + 27) = vAlloc;
		*(DWORD*)(loaderAlloc + 43) = newRVA + newSecSize + vmInit;
		*(DWORD*)(loaderAlloc + 52) = oldEntry;
		memmove(hNewMem + curPos, loaderAlloc, sizeof(loaderAlloc));
		curPos += sizeof(loaderAlloc);

		newSecSize += vmovmSize + sizeof(loaderAlloc);
	}
	DWORD oldNewSecSize = newSecSize;
	newSecSize = TRUNC(newSecSize, inh->OptionalHeader.FileAlignment);	

	//adding section	
	ish += inh->FileHeader.NumberOfSections;
	inh->FileHeader.NumberOfSections++;
	memset(ish, 0, sizeof(IMAGE_SECTION_HEADER));
	ish->Characteristics = 0xE00000E0;
	ish->Misc.VirtualSize = TRUNC(newSecSize, inh->OptionalHeader.SectionAlignment);
	inh->OptionalHeader.SizeOfImage += ish->Misc.VirtualSize;
	memmove(ish->Name, ".VM", 4);
	ish->PointerToRawData = fSize;
	ish->SizeOfRawData = newSecSize;
	ish->VirtualAddress = newRVA;

	char newFName[MAX_PATH] = {0};
	strcpy(newFName, fileName);
	newFName[strlen(newFName) - 4] = 0;
	strcat(newFName, "_vmed.exe");
	
	hFile = CreateFile(newFName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(hFile, hInMem, fSize, &tmp, 0);
	WriteFile(hFile, hNewMem, oldNewSecSize, &tmp, 0);
	memset(hNewMem, 0, newSecSize - oldNewSecSize);
	WriteFile(hFile, hNewMem, newSecSize - oldNewSecSize, &tmp, 0);
	CloseHandle(hFile);

	GlobalFree(hNewMem);
	GlobalFree(items);
	GlobalFree(items2);
	vm_free();
}

int WINAPI DialogProc(HWND hDlg, UINT uMSg, WPARAM wParam, LPARAM lParam)
{
	switch (uMSg)
	{
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			break;
		case WM_COMMAND:
			{
				switch (wParam & 0xFFFF)
				{
					case BTN_EXIT:
						EndDialog(hDlg, 0);
						break;
					case BTN_ADD:
						if (DialogBoxParam((HINSTANCE)0x400000, (LPCSTR)IDD_DIALOG2, hDlg, AddDialogProc, 0))
						{
							char temp[50];
							sprintf(temp, "%08X - %08X", ddFrom, ddTo);
							SendDlgItemMessage(hDlg, LB_LIST, LB_ADDSTRING, 0, (LPARAM)temp);
						}
						break;
					case BTN_OPEN:
						{
							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hwndOwner = hDlg;
							char fileName[MAX_PATH] = {0};
							ofn.lpstrFile = fileName;
							ofn.nMaxFile = MAX_PATH;
							ofn.Flags = OFN_FILEMUSTEXIST;
							GetOpenFileName(&ofn);
							SetDlgItemText(hDlg, EDT_FILE, fileName);
							SendDlgItemMessage(hDlg, LB_LIST, LB_RESETCONTENT, 0, 0);
						}
						break;
					case BTN_PROTECT:
						{
							char fileName[MAX_PATH];
							if (GetDlgItemText(hDlg, EDT_FILE, fileName, MAX_PATH))
							{
								bool vmovm = false;
								if (SendDlgItemMessage(hDlg, CHK_VMOVM, BM_GETCHECK, 0, 0) == BST_CHECKED) vmovm = true;
								doProtect(GetDlgItem(hDlg, LB_LIST), vmovm, fileName);
							}
							MessageBox(hDlg, "Finished.", "Info", MB_ICONINFORMATION);
						}
						break;
				}
			}
			break;
	}
	return 0;
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	DialogBoxParam(hInstance, (LPCSTR)IDD_DIALOG1, 0, DialogProc, 0);

	return (int)VirtualAlloc;
}