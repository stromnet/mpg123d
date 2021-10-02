/*
	GetOpt Class
*/

#include "GetOpt.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>


int GetOption (int argc, char** argv, char* pszValidOpts, char** ppszParam, bool bResetCounter)
{
    static int iArg = 1;
	
	if(bResetCounter)
		iArg=1;

    char chOpt;
    char* psz = NULL;
    char* pszParam = NULL;

    if (iArg < argc)
    {
        psz = &(argv[iArg][0]);
        if (*psz == '-' || *psz == '/')
        {
            // we have an option specifier
            chOpt = argv[iArg][1];
            if (isalnum(chOpt) || ispunct(chOpt))
            {
                // we have an option character
                psz = strchr(pszValidOpts, chOpt);
                if (psz != NULL)
                {
                    // option is valid, we want to return chOpt
                    if (psz[1] == ':')
                    {
                        // option can have a parameter
                        psz = &(argv[iArg][2]);
                        if (*psz == '\0')
                        {
                            // must look at next argv for param
                            if (iArg+1 < argc)
                            {
                                psz = &(argv[iArg+1][0]);
                                if (*psz == '-' || *psz == '/')
                                {
                                    // next argv is a new option, so param
                                    // not given for current option
                                }
                                else
                                {
                                    // next argv is the param
                                    iArg++;
                                    pszParam = psz;
                                }
                            }
                            else
                            {
                                // reached end of args looking for param
                            }

                        }
                        else
                        {
                            // param is attached to option
                            pszParam = psz;
                        }
                    }
                    else
                    {
                        // option is alone, has no parameter
                    }
                }
                else
                {
                    // option specified is not in list of valid options
                    chOpt = -1;
                    pszParam = &(argv[iArg][0]);
                }
            }
            else
            {
                // though option specifier was given, option character
                // is not alpha or was was not specified
                chOpt = -1;
                pszParam = &(argv[iArg][0]);
            }
        }
        else
        {
            // standalone arg given with no option specifier
            chOpt = 1;
            pszParam = &(argv[iArg][0]);
        }
    }
    else
    {
        // end of argument list
        chOpt = 0;
    }

    iArg++;
	if(ppszParam != NULL)
		*ppszParam = pszParam;
    else
        *ppszParam = NULL;
    
    return (chOpt);
}

bool IsArgAviable(int argc, char**argv, char cArg, char* lpszValidOptions)
{
	char *a=new char[2];
	char *b=new char[2];
	char cPrmParseRet;

	memset(a, 0, 2);
	memset(b, 0, 2);
	b[0]=cArg;

	cPrmParseRet = GetOption(argc, argv, lpszValidOptions, NULL, 1);

	do
	{
		a[0]=cPrmParseRet;

		if(!lstrcmp(CharLower(a),CharLower(b)))
		{
			delete a;
			delete b;
			return 1;
		}

	}while(cPrmParseRet=GetOption(argc, argv, lpszValidOptions, NULL,0));

	delete a;
	delete b;
	return 0;
}
