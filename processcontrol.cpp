//This is some application I made with help from the internet for personal purposes.. Nothing special..

#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <algorithm> 

using namespace std;
char cinProcessName[128];

//Functions 
int GetPID(char *pProcessName);
bool foundProcess( DWORD processID );
bool PrintMemoryInfo( DWORD processID );
void getProcessPoolInfo( char *processName );
int timer();
void initTimer( void );
char tryMonitorProcess(char *processName);
void getUserInput( void );
void checkForKeys( void );
void tryRunTasks( void );
int createMenuItem(char *charName, int times, int guitype, char *titleName);
void ClearScreen( void );
float returnProcessMemory(DWORD processID );
void pause();
void tryControlProcess( char *processName );
char* returnProcessPathInfo(DWORD processID);
void startProgram( char *lpApplicationName );
//Globals
clock_t t;
clock_t actualclock;
bool timerRunning;
float assignatedMemory;
bool processDown;
//Defines
#define GUI_TITLE 1
#define GUI_NORMAL 2
//------------------------------------------------------------------------------------------------------------------
// Prototype(s)...
//------------------------------------------------------------------------------------------------------------------
CHAR cpuusage(void);

//-----------------------------------------------------
typedef BOOL ( __stdcall * pfnGetSystemTimes)( LPFILETIME lpIdleTime, LPFILETIME lpKernelTime, LPFILETIME lpUserTime );
static pfnGetSystemTimes s_pfnGetSystemTimes = NULL;

static HMODULE s_hKernel = NULL;
//-----------------------------------------------------

//Entry
int main(void) {
    getUserInput();
	return 0;
}

void GetSystemTimesAddress() {
    if( s_hKernel == NULL ) {   
        s_hKernel = LoadLibrary( "Kernel32.dll" );
        if( s_hKernel != NULL ) {
            s_pfnGetSystemTimes = (pfnGetSystemTimes)GetProcAddress( s_hKernel, "GetSystemTimes" );
            if( s_pfnGetSystemTimes == NULL ) {
                FreeLibrary( s_hKernel ); s_hKernel = NULL;
            }
        }
    }
}

CHAR cpuusage() {
    FILETIME               ft_sys_idle;
    FILETIME               ft_sys_kernel;
    FILETIME               ft_sys_user;

    ULARGE_INTEGER         ul_sys_idle;
    ULARGE_INTEGER         ul_sys_kernel;
    ULARGE_INTEGER         ul_sys_user;

    static ULARGE_INTEGER    ul_sys_idle_old;
    static ULARGE_INTEGER  ul_sys_kernel_old;
    static ULARGE_INTEGER  ul_sys_user_old;

    CHAR  usage = 0;

    // we cannot directly use GetSystemTimes on C language
    /* add this line :: pfnGetSystemTimes */
    s_pfnGetSystemTimes(&ft_sys_idle,    /* System idle time */
        &ft_sys_kernel,  /* system kernel time */
        &ft_sys_user);   /* System user time */

    CopyMemory(&ul_sys_idle  , &ft_sys_idle  , sizeof(FILETIME)); // Could been optimized away...
    CopyMemory(&ul_sys_kernel, &ft_sys_kernel, sizeof(FILETIME)); // Could been optimized away...
    CopyMemory(&ul_sys_user  , &ft_sys_user  , sizeof(FILETIME)); // Could been optimized away...

    usage  =
        (
        (
        (
        (
        (ul_sys_kernel.QuadPart - ul_sys_kernel_old.QuadPart)+
        (ul_sys_user.QuadPart   - ul_sys_user_old.QuadPart)
        )
        -
        (ul_sys_idle.QuadPart-ul_sys_idle_old.QuadPart)
        )
        *
        (100)
        )
        /
        (
        (ul_sys_kernel.QuadPart - ul_sys_kernel_old.QuadPart)+
        (ul_sys_user.QuadPart   - ul_sys_user_old.QuadPart)
        )
        );

    ul_sys_idle_old.QuadPart   = ul_sys_idle.QuadPart;
    ul_sys_user_old.QuadPart   = ul_sys_user.QuadPart;
    ul_sys_kernel_old.QuadPart = ul_sys_kernel.QuadPart;

    return usage;
}
int GetPID(char *pProcessName) {
	HANDLE hSnap = INVALID_HANDLE_VALUE;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnap == INVALID_HANDLE_VALUE)
	return -1;
	if(Process32First(hSnap, &ProcessStruct) == FALSE)
	return -1;
	do {
		if(strcmp(ProcessStruct.szExeFile, pProcessName) == 0) {
			CloseHandle( hSnap );
			return ProcessStruct.th32ProcessID;
			break;
		}
	}
	while( Process32Next( hSnap, &ProcessStruct ) );
	CloseHandle( hSnap );
	return -1;
}
bool foundProcess( DWORD processID ) {
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;
	// Print information about the memory usage of the process.

	hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
									PROCESS_VM_READ,
									FALSE, processID );
	if (NULL == hProcess) {
		return 0;
	}
	CloseHandle( hProcess );
	return 1;
}
bool PrintMemoryInfo( DWORD processID ) {
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	// Print the process identifier.

	printf( "\nMemory Information on Process ID: %u\n", processID );
	printf("\n");
	// Print information about the memory usage of the process.

	hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
									PROCESS_VM_READ,
									FALSE, processID );
	if (NULL == hProcess) {
		return 0;
	}
	if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
		printf( "\tPageFaultCount: %.3f\n", floor(float(pmc.PageFaultCount)));
		printf( "\tCurrent memory usage (Approximate): %.3f\n", floor(float(pmc.WorkingSetSize/1024000 )));
		printf( "\tQuotaPeakPagedPoolUsage: %.3f\n", 
				floor(float(pmc.QuotaPeakPagedPoolUsage/1024000 )));
		printf( "\tQuotaPagedPoolUsage: %.3f\n", 
				floor(float(pmc.QuotaPagedPoolUsage/1024000 )));
		printf( "\tQuotaPeakNonPagedPoolUsage: %.3f\n", 
				floor(float(pmc.QuotaPeakNonPagedPoolUsage/1024000 )));
		printf( "\tQuotaNonPagedPoolUsage: %.3f\n", 
				floor(float(pmc.QuotaNonPagedPoolUsage/1024000 )));
		printf( "\tPagefileUsage: %.3f\n", 
				floor(float(pmc.PagefileUsage/1024000 ))); 
		printf( "\tPeakPagefileUsage: %.3f\n", 
				floor(float(pmc.PeakPagefileUsage/1024000 )));
	}

	CloseHandle( hProcess );
	return 1;
}
float returnProcessMemory(DWORD processID ) {
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
									PROCESS_VM_READ,
									FALSE, processID );
	if (NULL == hProcess) {
		return 0;
	}
	if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
		return floor(float(pmc.WorkingSetSize/1024000 ));
	}

	CloseHandle( hProcess );
	return 0;
}
int createMenuItem(char *charName, int times, int guitype, char *titleName) {
	int preCalcSize;
	char buffer[128];
	for(int i=0; i<times; i++) {
		printf("%s", charName);
		switch(guitype) {
			case GUI_TITLE: {
				if(i == 0) 
					continue;
				if(times/2 ==  i) {
					sprintf_s(buffer, " %s ", titleName);
					printf(buffer);
					preCalcSize = strlen(buffer) + times;
				}
			}
		}
	}
	printf("\n");
	return preCalcSize;
}
void getProcessPoolInfo( char *processName ) {
	char buffer[128];
	int size;
	sprintf_s(buffer, "Info on %s", processName);
	size = createMenuItem("-",45,GUI_TITLE,buffer);
	if(!foundProcess(GetPID(processName))) {
		printf("Couldn't find Process Name %s, please try re-opening it\n", processName);
		return;
	}
	printf("CPU Usage: %3d%%\r",cpuusage());
	PrintMemoryInfo(GetPID(processName));
	returnProcessPathInfo(GetPID(processName)) ? printf("Path: %s\n", returnProcessPathInfo(GetPID(processName))) : printf("Path: Couldn't return the path name\n");
	printf("Info: You can press SHIFT + ENTER at any time, to check for a different process.\n");
	createMenuItem("-",size,GUI_NORMAL,"");
}
char* returnProcessPathInfo(DWORD processID) {
	char buffer[MAX_PATH];
	HANDLE processHandle = NULL;
	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if (processHandle != NULL) {
		if (GetModuleFileNameEx(processHandle, NULL, buffer, MAX_PATH) == 0) {
			return 0;
		} else {
			return buffer;
		}
		CloseHandle(processHandle);
	} else {
		return 0;
	}
}
int timer() {
	actualclock = clock() - t;
	if(actualclock > 200) { //Every 200 ms
		tryRunTasks();
		t = clock(); //Last time
	}
	return 0;
}
void ClearScreen( void ) {
	HANDLE hndl = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hndl, &csbi);
	DWORD written;
	DWORD n = csbi.dwSize.X * csbi.dwCursorPosition.Y + csbi.dwCursorPosition.X + 1;	
	COORD curhome = {0,0};
	FillConsoleOutputCharacter(hndl, ' ', n, curhome, &written);
	csbi.srWindow.Bottom -= csbi.srWindow.Top;
	csbi.srWindow.Top = 0;
	SetConsoleWindowInfo(hndl, TRUE, &csbi.srWindow);
	SetConsoleCursorPosition(hndl, curhome);
}
void tryRunTasks( void ) {
	if(timerRunning) {
		ClearScreen();
		getProcessPoolInfo(cinProcessName);
		tryControlProcess(cinProcessName);
		checkForKeys();
	}
}
void TerminateProcessEx(DWORD processID) {
	HANDLE pHandle;
	pHandle = OpenProcess(PROCESS_ALL_ACCESS,false,processID);
	TerminateProcess(pHandle,1);
}
void tryControlProcess( char *processName ) {
	char buffer[MAX_PATH];
	if(returnProcessMemory(GetPID(processName)) > assignatedMemory && processDown == false) {
		sprintf_s(buffer,"%s",returnProcessPathInfo(GetPID(processName)));
		printf("The process %s has been restarted because it went past the assignated bounds (%f) on memory usage\n", processName, assignatedMemory);
		TerminateProcessEx(GetPID(processName));
		processDown = true;
		startProgram(buffer);
	}
}
void startProgram( char *lpApplicationName ) {
	/* Create the process */
	SHELLEXECUTEINFO sei = {0};
	sei.cbSize = sizeof(sei);
	sei.nShow = SW_SHOWNORMAL;
	sei.lpFile = TEXT(lpApplicationName);
	sei.fMask = SEE_MASK_CLASSNAME;
	sei.lpVerb = TEXT("open");
	sei.lpClass = TEXT("exefile");
	if(!ShellExecuteEx(&sei)) {
		printf("ShellExecute() failed to start program %s\n", lpApplicationName);
		pause();
		return;
		//exit(1);
	}
	processDown = false;
}
void initTimer( void ) {
	GetSystemTimesAddress();
	actualclock = 0;
	t = clock();
	for(;;) {
		timer();
	}
}
void checkForKeys( void ) {
	if(GetAsyncKeyState(VK_SHIFT) & GetAsyncKeyState(VK_RETURN)) {
		printf("The Enter and shift Key have both been Pressed\n");
		timerRunning = false;
		getUserInput();
	}
}
void pause() {
	std::cin.sync(); // Flush The Input Buffer Just In Case
	std::cin.ignore(); // There's No Need To Actually Store The Users Input
}
char tryMonitorProcess(char *processName) {
	if(!foundProcess(GetPID(cinProcessName))) {
		ClearScreen();
		printf("Couldn't find Process Name %s, please re-write it correctly as 'processname.exe'\n", cinProcessName);
		printf("Please, press any key to continue\n");
		pause();
		getUserInput();
	} else {
		timerRunning = true;
		initTimer();
	}
	return 0;
}
void getUserInput( void ) {
	ClearScreen();
	printf("Please enter the process name you're trying to monitor: ");
	cin>>cinProcessName;
	printf("Please enter the maximum amount of memory you're allowing that process to utilize: ");
	cin>>assignatedMemory;
	tryMonitorProcess(cinProcessName);
}
