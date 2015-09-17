#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>

typedef struct
{
	wchar_t		**Files;		// »м¤ файла
	int		*FileSize;		// Размер файла
	int			NumFiles,		// —колько файлов всего
				MaxFiles;		// ¬нутренн¤¤ переменна¤, дл¤ работы не используетс¤
	__int64		AllSize;		// Общий размер файлов, в байтах

	wchar_t **mPath;			// Подпапки
	int NumDir;					// Сколько всего папок
	int AllDir;					// Не в счет
} FileList_t;

INT_PTR FileList_Init( void );
void FileList_Free( FileList_t *List );
void FileList_Free( FileList_t *List );
BOOL SearchFiles( LPCTSTR lpszFileName, FileList_t *FileList,  BOOL bInnerFolders = FALSE, int StrLen = 0 );
bool CreateDirectoryTree(LPTSTR szDir);
wchar_t *GetFileName( const wchar_t *Str );
char *convertUnicode( const wchar_t* src );
int WaitWithMessageLoop( const HANDLE * hEvent, int count = 1 );

#endif		// __UTILS_H__
