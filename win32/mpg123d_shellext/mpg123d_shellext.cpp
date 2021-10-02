#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../../mpg123dc-lib/mpg123dc-lib.h"
#include "../GetOpt.h"

#define ARG_VALID_OPTIONS   "t:f:p:c:j:d"

#define DATATYPE_NONE   -1
#define DATATYPE_FILE   0
#define DATATYPE_FOLDER 1

#define ACTION_NONE     -1
#define ACTION_PLAY     0
#define ACTION_ENQUEUE  1
 
#define CONFIG_FILENAME     "\\mpg123dc.ini"

int FileExists(char*lpszFilename);
int FindConfigFile(char**lpszConfigFile);
void DebugMessage(int active, char*format, int extrabuff, ...);

/*
    
Parameters:
    -c [file]               Configuration File. Will search in Windows directory
                            and current path if not specifyed
    -p [path]               Folder name (if -t folder)
    -j [play|enqueue]       Action to do
    -t [file|folder]        Type of thing to use
    -f [filename]           Filename (if -t file)
    -d 						Debug mode
*/

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nShowCmd)
{
    // Start by parsing our cmdline
    char cParam;
    LPSTR lpszParameter =  new char [512];
    int debug=0;
    
    // We HAVE to initialize all our variables here... cause otherwise the GOTO will complian...
    int nDataType = DATATYPE_NONE;
    int nAction   = ACTION_NONE;
    LPSTR lpszFolderPath = NULL;
    LPSTR lpszFilePath = NULL;
    LPSTR lpszConfigFile = NULL;
    
    LPSTR lpszHost = NULL;
    int nPort = 0;
    LPSTR lpszNewPath = NULL;
    
    // Create our client-object
    MPG123DC myClient;

    // Parse the cmdline
    while((cParam = GetOption(__argc, __argv, ARG_VALID_OPTIONS, &lpszParameter, 0)))
    {
        switch(cParam)
        {
                case 't':
              			if(!stricmp(lpszParameter, "file"))
              				nDataType = DATATYPE_FILE;
   					    else if(!stricmp(lpszParameter, "folder"))
   					        nDataType = DATATYPE_FOLDER;
               			break;
                case 'f':
                        lpszFilePath = new char[lstrlen(lpszParameter)+1];
                        lstrcpy(lpszFilePath, lpszParameter);
                        DebugMessage(debug, "File to add %s", lstrlen(lpszFilePath), lpszFilePath);
                        break;
                case 'p':
                        lpszFolderPath = new char[lstrlen(lpszParameter)+1];
                        lstrcpy(lpszFolderPath, lpszParameter);
                        DebugMessage(debug, "Folder to add %s", lstrlen(lpszFolderPath), lpszFolderPath);
                        break;
                case 'c':
                        lpszConfigFile = new char[lstrlen(lpszParameter)+1];
                        lstrcpy(lpszConfigFile, lpszParameter);
                        DebugMessage(debug, "Setting configuration file to %s", lstrlen(lpszConfigFile), lpszConfigFile);
                        break;
                case 'j':
                        if(!stricmp(lpszParameter, "play"))
              				nAction = ACTION_PLAY;
   					    else if(!stricmp(lpszParameter, "enqueue"))
   					        nAction = ACTION_ENQUEUE;
               			break;
       			case 'd':
       					debug=1;
       					break;
                default:                 // err...
                		DebugMessage(debug, "Err, invalid parameter %c... Exiting", 0, cParam);
                        goto CleanupAndDelete;
                        return 1;
        }
    }
    DebugMessage(debug, "Done parsing params.. Making checks..", 0);

    // Check for nDataType and lpszFilePath or lpszFolderPath
    if( nDataType == DATATYPE_NONE ||
        ( nDataType == DATATYPE_FILE && !lpszFilePath) ||
        ( nDataType == DATATYPE_FOLDER && !lpszFolderPath)
      )
        goto CleanupAndDelete;
	
	DebugMessage(debug, "nDataType is %d", 0, nDataType);

    // Check for action
    if( nAction == ACTION_NONE)
        goto CleanupAndDelete;

	DebugMessage(debug, "nAction is %d", 0, nAction);
	
    // Now parse configuration file
    if(!lpszConfigFile)
    {
    	DebugMessage(debug, "No configuration files specified as parameter... Checking default ones", 0);
        // No config file specified? find it..
        if(!FindConfigFile(&lpszConfigFile)) 
        {
        	DebugMessage(debug, "No default found.. Exiting", 0);
            // No cfg file found
            goto CleanupAndDelete;
        }
        
        DebugMessage(debug, "Using config file %s", lstrlen(lpszConfigFile), lpszConfigFile);
        // File is now placed in lpszConfigFile..
    }

    // Alloc memory for host
    lpszHost = new char[255];
    
    DebugMessage(debug, "Retrieving host/port from configfile", 0);
    
    // Read host settings...
    GetPrivateProfileString("mpg123d", "host", "", lpszHost, 255, lpszConfigFile);
    nPort = GetPrivateProfileInt("mpg123d", "port", 3322, lpszConfigFile);

	DebugMessage(debug, "Found host %s and port %d", lstrlen(lpszHost)+10, lpszHost, nPort);
	
    // Now do path translation
    if(nDataType == DATATYPE_FILE)
        myClient.doPathTranslation(lpszFilePath, &lpszNewPath, lpszConfigFile);
    else
        myClient.doPathTranslation(lpszFolderPath, &lpszNewPath, lpszConfigFile);

	DebugMessage(debug, "Path/file after translation: %s", lstrlen(lpszNewPath), lpszNewPath);
    
    if(!lpszNewPath)
        goto CleanupAndDelete;

	DebugMessage(debug, "Setting up socket.. ", 0);
    // Set it up and connect...
    if(myClient.setupSocket(lpszHost, nPort))
        goto CleanupAndDelete;
    
    DebugMessage(debug, "Sending commands", 0);
    // empty playlist first if action is play
    if(nAction == ACTION_PLAY)
        myClient.playlist_Clear();

    // Send cmd
    if(nDataType == DATATYPE_FILE)
        myClient.playlist_addFile(lpszNewPath, MPG123DC_POS_END);
    else
        myClient.playlist_addDir(lpszNewPath, MPG123DC_POS_END);
    
    // play music if action is play
    if(nAction == ACTION_PLAY)
        myClient.player_Play();
    
    // Cleanup and exit... we are done    
CleanupAndDelete:
	DebugMessage(debug, "Cleaning up", 0);
    if(lpszFilePath) delete lpszFilePath;
    if(lpszFolderPath) delete lpszFolderPath;
    if(lpszConfigFile) delete lpszConfigFile;
    if(lpszHost) delete lpszHost;
    if(lpszNewPath) delete lpszNewPath;

	DebugMessage(debug, "Exiting", 0);
    return 0; 
}

int FileExists(char*lpszFilename)
{
    HANDLE hFile;
    
    hFile = CreateFile(lpszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
       
    if(hFile == INVALID_HANDLE_VALUE)
        return 0;   // File doesnt exist
    
    // File exists
    CloseHandle(hFile);
    return 1;
}

/* Caller are responsible for deleting lpszConfigFile if allocated...... */
int FindConfigFile(char**lpszConfigFile)
{
    LPSTR lpszBuffer = new char[512];
    
    // First check in windows dir
    GetWindowsDirectory(lpszBuffer, 512);
    lstrcat(lpszBuffer, CONFIG_FILENAME);
    
    if(FileExists(lpszBuffer))
    {
        *lpszConfigFile = new char[lstrlen(lpszBuffer)+1];
        lstrcpy(*lpszConfigFile, lpszBuffer);
        delete lpszBuffer;
        return 1;
    }
    
    // Check in current directory
    GetCurrentDirectory(512, lpszBuffer);
    lstrcat(lpszBuffer, CONFIG_FILENAME);
    
    if(FileExists(lpszBuffer))
    {
        *lpszConfigFile = new char[lstrlen(lpszBuffer)+1];
        int x=GetLastError();
        lstrcpy(*lpszConfigFile, lpszBuffer);
        delete lpszBuffer;
        return 1;
    }
    
    delete lpszBuffer;
    return 0;
}

void DebugMessage(int active, char*format, int extrabuff, ...)
{
	if(!active) return ;
	
	char *buffer = new char [lstrlen(format) + extrabuff + 10];
	
	va_list pa;
	va_start(pa, extrabuff);
	
	wvsprintf(buffer, format, pa);
	
	MessageBox(NULL, buffer, "Debug message", MB_OK);
	
	delete buffer;
	va_end(pa);
}
