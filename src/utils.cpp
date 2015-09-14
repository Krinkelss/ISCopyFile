#include "utils.h"
#include <tchar.h>

int GetFileSize( LPCWSTR File )
{
	int nFileLen = 0;
	WIN32_FILE_ATTRIBUTE_DATA fData;
	int res = GetFileAttributesEx( File, GetFileExInfoStandard, &fData );
	if( res )
		nFileLen = ( fData.nFileSizeHigh * ( MAXDWORD + 1 ) ) + fData.nFileSizeLow;

	return nFileLen;
}

// Выделяем память под массив файлов
INT_PTR FileList_Init( void )
{
	FileList_t	*List;
	List = ( FileList_t * )malloc( sizeof( FileList_t ) );
	List->NumFiles = 0;
	List->MaxFiles = 10;
	List->AllSize = 0;
	List->Files = ( wchar_t ** )calloc( List->MaxFiles, sizeof( wchar_t ** ) );

	List->FileSize = ( int * )calloc( List->MaxFiles, sizeof( int ) );

	List->NumDir = 0;
	List->AllDir = 5;
	List->mPath = ( wchar_t ** )calloc( List->AllDir, sizeof( wchar_t ** ) );

	return ( INT_PTR )List;
}

void mPath_Add( FileList_t *List, wchar_t *Str, int Len )
{
	wchar_t 	**NewP;

	if( List->NumDir == List->AllDir )
	{
		List->AllDir += 5;
		NewP = (wchar_t **) realloc( List->mPath, List->AllDir * sizeof( wchar_t ** ) );
		if( !NewP )
		{
			MessageBox( NULL, L"Ошибка при выделении памяти", L"Внимание!", MB_ICONERROR );
			return;
		}
		List->mPath = NewP;
	}

	wchar_t *Tmp = Str + Len - 1;

	List->mPath[ List->NumDir ] = _wcsdup( Tmp );
	List->NumDir++;
}

//Добавляем строки в массив строк
void FileList_Add( FileList_t *List, wchar_t *Path )
{
	int		NumFiles = List->NumFiles;
	wchar_t 	**NewList;
	int		*NewSize;
	int		FileSize;

	if ( NumFiles == List->MaxFiles )
	{
		List->MaxFiles = List->MaxFiles + 10;
		NewList = (wchar_t **) realloc( List->Files, List->MaxFiles * sizeof( wchar_t ** ) );
		if( !NewList )
		{
			MessageBox( NULL, L"Ошибка при выделении памяти", L"Внимание!", MB_ICONERROR );
			return;
		}
		List->Files = NewList;

		NewSize = ( int * )realloc( List->FileSize, List->MaxFiles * 8 );
		if( !NewSize )
		{
			MessageBox( NULL, L"Ошибка при выделении памяти", L"Внимание!", MB_ICONERROR );
			return;
		}
		List->FileSize = NewSize;		
	}

	FileSize = GetFileSize( Path );

	List->Files[ NumFiles ] = _wcsdup( Path );

	List->FileSize[ NumFiles ] = FileSize;

	List->AllSize += FileSize;
	List->NumFiles++;
}

// Удаляем память
void FileList_Free( FileList_t *List )
{
	int i;
	if ( !List )
		return;

	for ( i = 0; i < List->NumFiles; i++ )
		free( List->Files[ i ] );

	for( int i = 0; i < List->NumDir; ++i )
		free( List->mPath[ i ] );
	free( List->mPath );

	free( List->FileSize );
	free( List->Files );
	free( List );	
}

BOOL SearchFiles( LPCTSTR lpszFileName, FileList_t *FileList,  BOOL bInnerFolders, int StrLen )
{
	LPTSTR part;
	wchar_t tmp[ _MAX_PATH ] = {0};
	wchar_t name[ _MAX_PATH ] = {0};
	wchar_t file[ _MAX_PATH ] = {0};

	HANDLE hSearch = NULL;
	WIN32_FIND_DATA wfd;
	memset( &wfd, 0, sizeof( WIN32_FIND_DATA ) );
	
	if( bInnerFolders )
	{
		if( GetFullPathNameW( lpszFileName, MAX_PATH, tmp, &part ) == 0 ) return FALSE;
		lstrcpyW( name, part );
		lstrcpyW( part, L"*.*" );

		wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		if ( !( ( hSearch = FindFirstFileW( tmp, &wfd ) ) == INVALID_HANDLE_VALUE ) )
			do
			{
				if ( !wcsncmp( wfd.cFileName, L".", 1 ) || !wcsncmp( wfd.cFileName, L"..", 2 ) )
					continue;

				if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					wchar_t next[ _MAX_PATH ] = {0};
					if( GetFullPathNameW( lpszFileName, MAX_PATH, next, &part ) == 0 ) return FALSE;
					lstrcpyW( part, wfd.cFileName );
					lstrcatW( next, L"\\" );					
					
					mPath_Add( FileList, next, StrLen );
					
					lstrcatW( next, name );
										
					SearchFiles( next, FileList, TRUE, StrLen );
				}
			}
			while ( FindNextFileW( hSearch, &wfd ) );

			FindClose ( hSearch );
	}

	if ( ( hSearch = FindFirstFileW( lpszFileName, &wfd ) ) == INVALID_HANDLE_VALUE )
		return TRUE;
	do
	if ( !( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		wchar_t file[ _MAX_PATH ] = {0};
		if( GetFullPathNameW( lpszFileName, _MAX_PATH, file, &part ) == 0 ) return FALSE;
		lstrcpyW( part, wfd.cFileName );
		
		FileList_Add( FileList, file );
	}
	while ( FindNextFileW( hSearch, &wfd ) );
	FindClose ( hSearch );
	return TRUE;
}

bool CreateDirectoryTree( wchar_t *szDir )
{
	// Проверяем, существует ли директория
	DWORD dwAttr = GetFileAttributes( szDir );
	if( dwAttr != INVALID_FILE_ATTRIBUTES && ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) )
		return true;

	wchar_t* pszSlash = wcsrchr(szDir, TEXT('\\'));
	if (pszSlash == NULL)
		return false;

	*pszSlash = 0;
	bool res = CreateDirectoryTree(szDir);
	*pszSlash = TEXT('\\');

	if( res ) res = CreateDirectory(szDir, NULL) != 0;

	return res;
}

wchar_t *GetShortFileName( wchar_t *FileNames )
{
	static wchar_t aTmp[ _MAX_PATH ] = {0};

	if ( GetShortPathName( FileNames, aTmp, sizeof( aTmp ) / sizeof( wchar_t ) ) )
		return ( wchar_t * )aTmp;
	else
		return ( wchar_t * )FileNames;
}

wchar_t *GetExeName( const wchar_t *Str )
{
	wchar_t name[ _MAX_FNAME ] = {0};
	wchar_t ext[ _MAX_EXT ] = {0};

	static wchar_t Result[ _MAX_PATH ] = {0};

	_wsplitpath( Str, NULL, NULL, name, ext );
	wsprintf( Result, L"%s%s", name, ext );
			
	return ( wchar_t * )GetShortFileName( Result );
}