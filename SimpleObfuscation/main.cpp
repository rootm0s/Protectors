/*
*   Copyright © May-30-2010 by learn_more
*   main.cpp is part of the project 'SimpleObfuscation'.
*
*   Please do not use this in payhacks.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY, without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*$UCC_HDR$*
*/


#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0600
#include "windows.h"
#include <detours.h>
#include <iostream>

#include "SimpleObf.h"

/*
	Example usage functions for my obfuscation library,
	possibilities:
      *  Inserting number of nops that get replaced (on runtime) with different opcodes), usefull for pattern evasion
      *  Create a randomized callgate, meaning a detoured function gets pointed to the callgate, it fills the specified number of instructions with nop equivalents, and then jumps to your function. (usefull to escape pb's hook blacklists, they tend to scan what is behind a hook and blacklist known patterns)
      *  Erase after execution, usefull for example when initializing hooks, a menu or whatever

*/


int (WINAPI* oMessageBoxA)( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType );

__declspec(noinline) 
int WINAPI myMessageBoxA( HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType )
{
RUNONCE_START(1);	//the RUNONCE_START argument must match that of the accompanying RUNONCE_END
					//nesting these macro's is not recommended
	lpText = "Only the first time the message will be changed";
RUNONCE_END(1);
	return oMessageBoxA( hWnd, lpText, lpCaption, uType );
}



//PBYTE that holds the jumpgate
PBYTE MsgJump;

int main()
{
	MsgJump = AllocateJumpgate( (PBYTE)myMessageBoxA, 20, 30 );		//allocate the jumpgate :)
		//the jumpgate needs a target function, which it will jump to after (min,max) randomized instructions

	PBYTE pOrMsg = (PBYTE)GetProcAddress(LoadLibraryA("user32"),"MessageBoxA");

	*(void**)&oMessageBoxA = (void*)DetourFunction( pOrMsg, MsgJump );		//detour with the jumpgate as target!!
	MessageBoxA( NULL, "Message", "Original caption", MB_OK );		//now the call will go trough the jumpage first, then myMessageBox
	MessageBoxA( NULL, "Message", "Original caption", MB_OK );		//now the call will go trough the jumpage first, then myMessageBox
	
	OBFUSCATENOPS(NOP5);		//the previously seen obfuscation, just makes it harder for patterns,
								//this insert number of nops (see .h for available macro's) and on first execution they are
								//replaced with randomized instructions that equal nop

	OBFUSCATENOPS(NOP5);

	int mVar = 5;
	std::cout << "pre loop: " << mVar << std::endl;
	for( int n = 0; n < 4; n++ )
	{
		RUNONCE_START(1);		//new macro, everything between start and end will execute once, then its erased (randomized data)
		mVar += 5;
		RUNONCE_END(1);			//the number needs to be unique inside a function,
								//you can use a text aswell (aslong as there are no spaces, and no quotes around it
		std::cout << "in loop: " << mVar << std::endl;
	}
	RUNONCE_START(my_erase_identifier);
	DetourRemove( (PBYTE)oMessageBoxA, pOrMsg );
	FreeJumpgate( MsgJump );
	RUNONCE_END(my_erase_identifier);
	std::cin.get();
}
