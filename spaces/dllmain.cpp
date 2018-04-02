// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <iostream>
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	int t;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//int t;
		//std::cin >> t;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		std::cin >> t;	
		break;
	}
	return TRUE;
}

