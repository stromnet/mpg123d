/*
	GetOpt.h
*/

#include <windows.h>

int GetOption (int argc, char** argv, char* pszValidOpts, char** ppszParam, bool bResetCounter);
bool IsArgAviable(char cArg, char* lpszValidOptions);

