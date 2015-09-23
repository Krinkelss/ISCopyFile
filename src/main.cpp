#include <windows.h>
#include "utils.h"
#include <stdio.h>

#pragma warning( disable : 4995 ) // Отключаем ошибку типа "warning C4995: wcscpy: имя помечено как #pragma deprecated"

#pragma comment(linker, "/ENTRY:DllMain")

typedef int ( __stdcall *FileCopyCallback_t )( char *what, int int1, char *Str );

//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <strsafe.h>

//BOOL mCopy;

HANDLE hEvent=NULL, hThread=NULL;
DWORD thID=NULL;
FileList_t *FileList;
wchar_t mPathOut[ _MAX_PATH ] = {0};
int SizePathOut;
BOOL mbInnerFolders;
char ConvertPath[ _MAX_PATH ] = {0};

void ErrorExit(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"), 
		lpszFunction, dw, lpMsgBuf); 
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
//////////////////////////////////////////////////////////////////////////
void __stdcall BreakCopy( void )
{
	mCopy = FALSE;	
}

DWORD WINAPI SearchThread (LPVOID IpParam)
{	
	SearchFiles( mPathOut, FileList, mbInnerFolders, SizePathOut );
	hThread = NULL;
	SetEvent( hEvent );
	return 0;
}

void __stdcall isCopyFile( FileCopyCallback_t callback, wchar_t *PathOut, wchar_t *PathIn, BOOL bInnerFolders )
{	
	wchar_t TempDir[ _MAX_PATH ] = {0};
	__int64 mCopySize = 0;
	FILE *mFileOut;		// Из
	FILE *mFileIn;		// В
	int PolKilo = 1024 * 512;
	wchar_t *TempBuf;
	size_t result;

	mCopy = TRUE;
		
	FileList = ( FileList_t * )FileList_Init();

	TempBuf = ( wchar_t * )malloc( PolKilo );
	SizePathOut = wcslen( PathOut );
	mbInnerFolders = bInnerFolders;
	wcscpy( mPathOut, PathOut );
	
	// Ищем файлы
	SECURITY_ATTRIBUTES sa;
	ZeroMemory (&sa, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hEvent = CreateEvent (&sa, false, false, NULL);
	hThread = CreateThread (&sa, NULL, SearchThread, NULL, NULL, &thID);

	WaitWithMessageLoop( &hEvent );
	
	CloseHandle( hEvent );
	CloseHandle( hThread );

	if( !mCopy )
	{
		free( TempBuf );
		FileList_Free( FileList );
		return;
	}

	FileList->AllSize = FileList->AllSize / 1024 ;
	
	callback( "allsize", ( int )FileList->AllSize, "" );
		
	// Создаем папку
	CreateDirectoryTree( PathIn );

	// Создаем подпапки
	for( int i = 0; i < FileList->NumDir; ++i )
	{
		wsprintf( TempDir, L"%s\\%s", PathIn, FileList->mPath[ i ] );		
		CreateDirectoryTree( TempDir );
	}

	for( int z = 0; z < FileList->NumFiles; ++z )
	{
		if( !mCopy )
			break;

		// Создаем файл в "конечной" папке
		if( bInnerFolders )
			wsprintf( TempDir, L"%s\\%s", PathIn, FileList->Files[ z ] + SizePathOut - 1 );
		else
			wsprintf( TempDir, L"%s\\%s", PathIn, GetFileName( FileList->Files[ z ] ) );
		mFileIn = _wfopen( TempDir, L"wb" );
		if( mFileIn == NULL )
			break;

		callback( "filename", 0, convertUnicode( FileList->Files[ z ] ) );
		
		// Открываем исходный файл
		mFileOut = _wfopen( FileList->Files[ z ], L"rb" );
		if( mFileIn == NULL )
			break;

		while( !feof( mFileOut ) && mCopy )
		{
			result = fread( TempBuf, 1, PolKilo, mFileOut );
			if( ferror( mFileOut ) )
			{
				ErrorExit( L"fread" );
				fclose( mFileOut );
				fclose( mFileIn );
				continue;
			}

			mCopySize += result;

			fwrite( TempBuf, 1, result, mFileIn );
			if( ferror( mFileIn ) )
			{				
				ErrorExit( L"fread" );
				fclose( mFileOut );
				fclose( mFileIn );
				continue;
			}
			callback( "write", mCopySize / 1024, "" );
		}
		
		fclose( mFileOut );
		fclose( mFileIn );
	}
	free( TempBuf );

	FileList_Free( FileList );
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	return TRUE;
}
