#include "common.h"
#include <stdio.h>
#include "../GetOpt.h"

#define ARG_VALID_OPTIONS   "dc:"
#define CONFIG_FILENAME     "\\mpg123dc.ini"
#define VERSION				"Beta v0.2"

char g_lpszClassName[] = "mgp132d_client_wndclass";
LPSTR lpszConfigFile = NULL;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

NOTIFYICONDATA g_nifData;
MPG123DC myClient;

int FileExists(char*lpszFilename);
int FindConfigFile(char**lpszConfigFile);
void DebugMessage(int active, char*format, int extrabuff, ...);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nShowCmd)
{
    char cParam;
    int debug=0;
    LPSTR lpszParameter =  new char [512];

    // Parse the cmdline
    while((cParam = GetOption(__argc, __argv, ARG_VALID_OPTIONS, &lpszParameter, 0)))
    {
        switch(cParam)
        {
                case 'c':
                        lpszConfigFile = new char[lstrlen(lpszParameter)+1];
                        lstrcpy(lpszConfigFile, lpszParameter);
                        DebugMessage(debug, "Setting configuration file to %s", lstrlen(lpszConfigFile), lpszConfigFile);
                        break;
       			case 'd':
       					debug=1;
       					break;
                default:                 // err...
                		DebugMessage(debug, "Err, invalid parameter %c... Exiting", 0, cParam);
                		if(lpszConfigFile) delete lpszConfigFile;
                        delete lpszParameter;
                        return 1;
        }
    }
    
    delete lpszParameter;
    
    DebugMessage(debug, "Done parsing params.. Making checks..", 0);
    
    // Now parse configuration file
    if(!lpszConfigFile)
    {
    	DebugMessage(debug, "No configuration files specified as parameter... Checking default ones", 0);
        // No config file specified? find it..
        if(!FindConfigFile(&lpszConfigFile)) 
        {
        	DebugMessage(debug, "No default found.. Exiting", 0);
            // No cfg file found
            delete lpszConfigFile;
            return 1;
        }
        
        DebugMessage(debug, "Using config file %s", lstrlen(lpszConfigFile), lpszConfigFile);
        // File is now placed in lpszConfigFile..
    }
	
    // First create a simple window..
    WNDCLASS wndCls;
    HWND hWnd;
    
    wndCls.style = CS_VREDRAW|CS_HREDRAW;
    wndCls.lpfnWndProc = WindowProc;
    wndCls.cbClsExtra = 0;
    wndCls.cbWndExtra = 0;
    wndCls.hInstance = hInstance;
    wndCls.hIcon = NULL;
    wndCls.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndCls.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wndCls.lpszMenuName = NULL;
    wndCls.lpszClassName = g_lpszClassName;
    
    if(!RegisterClass(&wndCls))
    {
        MessageBox(NULL, "Failed to register class...", "Errr", MB_OK);
        delete lpszConfigFile;
        return 1;
    }
    
    hWnd = CreateWindow(g_lpszClassName, "", 0, 0,0,0,0,0,NULL,hInstance, NULL);
    if(!hWnd)
    {
        MessageBox(NULL, "Failed to create window...", "Errr", MB_OK);
		delete lpszConfigFile;
        return 1;
    }
    
    g_nifData.cbSize = sizeof(NOTIFYICONDATA);
    g_nifData.hWnd = hWnd;
    g_nifData.uID=0;
    g_nifData.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP;
    g_nifData.uCallbackMessage = WM_MOUSEMOVE;
    g_nifData.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    
    lstrcpyn(g_nifData.szTip, "MPG123d Client "VERSION, 64);
    
    Shell_NotifyIcon(NIM_ADD, &g_nifData);

    // Alloc memory for host
    LPSTR lpszHost = new char[255];
    int nPort;
    
    DebugMessage(debug, "Retrieving host/port from configfile", 0);
    
    // Read host settings...
    GetPrivateProfileString("mpg123d", "host", "localhost", lpszHost, 255, lpszConfigFile);
    nPort = GetPrivateProfileInt("mpg123d", "port", 3322, lpszConfigFile);

	DebugMessage(debug, "Using host %s and port %d", lstrlen(lpszHost)+10, lpszHost, nPort);
	
	myClient.setupSocket(lpszHost, nPort);

	delete lpszHost;
	
    MSG mMsg;
    while(GetMessage(&mMsg, 0, 0, 0))
    {
        TranslateMessage(&mMsg);
        DispatchMessage(&mMsg);
    }
    
    Shell_NotifyIcon(NIM_DELETE, &g_nifData);

	delete lpszConfigFile;
     
    return 1; 
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE: 
                PostQuitMessage(0);
                break;
        case WM_MOUSEMOVE:      // This will react when the user moves the mouse over the tray-icon
                if(lParam == WM_RBUTTONDOWN)
                {
                    // Create menu and show it
                    HMENU hMenu = CreatePopupMenu();
                    POINT pnt;
                    
                    GetCursorPos(&pnt);
                    
                    // Workaround for MS bug, documented in MS KB Q135788 (http://support.microsoft.com/default.aspx?scid=KB;en-us;q135788)
                    SetForegroundWindow(hWnd);
                    
                    AppendMenu(hMenu, MF_STRING, IDM_PLAY, "&Play");
                    AppendMenu(hMenu, MF_STRING, IDM_PAUSE, "P&ause");
                    AppendMenu(hMenu, MF_STRING, IDM_PREV, "P&rev");
                    AppendMenu(hMenu, MF_STRING, IDM_NEXT, "&Next");
                    AppendMenu(hMenu, MF_STRING, IDM_STOP, "&Stop");
                    AppendMenu(hMenu, MF_STRING, IDM_STATUS, "S&tatus");
                    AppendMenu(hMenu, MF_SEPARATOR, 0, "");
                    AppendMenu(hMenu, MF_STRING, IDM_QUIT, "&Quit");
                    
                    TrackPopupMenuEx(hMenu, 0, pnt.x, pnt.y, hWnd, NULL);
                    
                    // Part 2 of the bugfix...
                    PostMessage(hWnd, WM_NULL, 0, 0);
                    
                    DestroyMenu(hMenu);
                }
                break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDM_QUIT:		PostQuitMessage(0);		break;
                case IDM_PLAY:		myClient.player_Play(0);break;
                case IDM_PAUSE:		myClient.player_Pause();break;
                case IDM_PREV:		myClient.player_Prev(1);break;
                case IDM_NEXT:		myClient.player_Next(1);break;
                case IDM_STOP:		myClient.player_Stop();	break;
                case IDM_STATUS:
                {
                 	int retval = myClient.player_getState();
                 	
                 	if(retval > 0)
                 	{
                 		LPSTR lpszBuff=new char[500];

                 		sprintf(lpszBuff, "Played %.1f seconds from %s by %s, from album %s (%s). Comment is %s and genre is %s. %.1f seconds left. Track is sampled at %d Hz, and has bitrate %d",
                   							myClient.timePlayed, myClient.lpszTitle, myClient.lpszArtist, myClient.lpszAlbum, myClient.lpszYear,
                          					myClient.lpszComment, myClient.lpszGenre, myClient.timeLeft, myClient.frequency, myClient.bitrate);

                 		MessageBox(NULL, lpszBuff, "Status", MB_OK);
                 		                   		
                   		delete lpszBuff;
                 	}
                 	
                 	break;
             	}
            }
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
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
