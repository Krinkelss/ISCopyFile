#include <windows.h>
#include "utils.h"
#include <stdio.h>
#include "resource.h"

#pragma comment(linker, "/ENTRY:DllMain")

typedef int ( __stdcall *FileCopyCallback_t )( int AllSize, int mCopySize, wchar_t *Str );

//////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <strsafe.h>

BOOL mCopy = TRUE;

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
void __stdcall BreakCopy( BOOL fCopy )
{
	mCopy = fCopy;
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
	FileList_t *FileList;
		
	FileList = ( FileList_t * )FileList_Init();

	TempBuf = ( wchar_t * )malloc( PolKilo );
	int SizePathOut = wcslen( PathOut );

	// Ищем файлы
	SearchFiles( PathOut, FileList, bInnerFolders, SizePathOut );

	FileList->AllSize = FileList->AllSize / 1024 ;
		
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
			wsprintf( TempDir, L"%s\\%s", PathIn, GetExeName( FileList->Files[ z ] ) );

		mFileIn = _wfopen( TempDir, L"wb" );
		if( mFileIn == NULL )
			break;

		// Открываем исходный файл
		mFileOut = _wfopen( FileList->Files[ z ], L"rb" );
		if( mFileIn == NULL )
			break;

		while( !feof( mFileOut ) && mCopy )
		{
			result = fread( TempBuf, 1, PolKilo, mFileOut );
			if( ferror( mFileOut ) )
			{
				//MessageBox( 0, L"Error", NULL, IDOK );
				ErrorExit( L"fread" );
				fclose( mFileOut );
				fclose( mFileIn );
				continue;
			}

			mCopySize += result;

			fwrite( TempBuf, 1, result, mFileIn );
			if( ferror( mFileIn ) )
			{
				ErrorExit( L"fwrite" );
				fclose( mFileOut );
				fclose( mFileIn );
				continue;
			}
			callback( ( int )FileList->AllSize, mCopySize / 1024, FileList->Files[ z ] );
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