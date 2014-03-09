//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _STRING_FUNCTIONS_H_
#define _STRING_FUNCTIONS_H_

#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <cstdio>

#if defined(TORQUE_OS_WIN32)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#if defined(TORQUE_COMPILER_CODEWARRIOR)
#define strupr _strupr
#endif

//------------------------------------------------------------------------------

inline U32 dStrlen(const char *str)
{
   return (U32) strlen(str);
}

inline char* dStrcat(char *dst, const char *src)
{
   return strcat(dst, src);
}
//extern UTF8* dStrcat(UTF8 *dst, const UTF8 *src);

inline char* dStrncat(char* dst, const char* src, dsize_t len)
{
   return strncat(dst, src, len);
}

extern char* dStrcatl(char *dst, dsize_t dstSize, ...);

inline int dStrcmp(const char *str1, const char *str2)
{
   return strcmp(str1, str2);
}

extern int dStrcmp(const UTF16 *str1, const UTF16 *str2);
// extern int dStrcmp(const UTF8 *str1, const UTF8 *str2);

extern int dStricmp(const char *str1, const char *str2);
extern int dStrncmp(const char *str1, const char *str2, dsize_t len);
extern int dStrnicmp(const char *str1, const char *str2, dsize_t len);

extern char* dStrcpy(char *dst, const char *src);
extern char* dStrcpyl(char *dst, dsize_t dstSize, ...);
extern char* dStrncpy(char *dst, const char *src, dsize_t len);
// extern char* dStrncpy(UTF8 *dst, const UTF8 *src, dsize_t len);

extern char* dStrupr(char *str);
extern char* dStrlwr(char *str);

inline char dToupper(const char c)
{
   return toupper(c);
}

inline char dTolower(const char c)
{
   return tolower(c);
}

inline char* dStrchr(char *str, int c)
{
   return strchr(str, c);
}

inline const char* dStrchr(const char *str, int c)
{
   return strchr(str, c);
}

inline char* dStrrchr(char *str, int c) {
   return strrchr(str, c);
}

inline const char* dStrrchr(const char *str, int c) {
   return strrchr(str, c);
}

inline U32 dStrspn(const char *str, const char *set)
{
   return (U32) strspn(str, set);
}

inline U32 dStrcspn(const char *str, const char *set)
{
   return (U32) strcspn(str, set);
}

extern char* dStrstr(char *str1, char *str2);
extern char* dStrstr(const char *str1, const char *str2);

extern const char* dStristr( const char* str1, const char* str2 );
extern char* dStristr( char* str1, const char* str2 );

inline char* dStrtok(char *str, const char *sep)
{
   return strtok(str, sep);
}

extern int dStrrev(char* str);

inline int dAtoi(const char *str)
{
   return atoi(str);
}

inline float dAtof(const char *str)
{
   return atof(str);
}

extern bool dAtob(const char *str);
extern int dItoa(int n, char s[]);

inline bool dIsalnum(const char c)
{
   return isalnum(c) != 0;
}

inline bool dIsalpha(const char c)
{
   return isalpha(c) != 0;
}

inline bool dIsdigit(const char c)
{
   return isdigit(c) != 0;
}

inline bool dIsspace(const char c)
{
   return isspace(c) != 0;
}

extern int dSscanf(const char *buffer, const char *format, ...);

inline int dFflushStdout()
{
   return fflush(stdout);
}

inline int dFflushStderr()
{
   return fflush(stderr);
}

extern void dPrintf(const char *format, ...);
extern int dVprintf(const char *format, va_list arglist);
extern int dSprintf(char *buffer, dsize_t bufferSize, const char *format, ...);
extern int dVsprintf(char *buffer, dsize_t bufferSize, const char *format, va_list arglist);

#define QSORT_CALLBACK FN_CDECL
extern void dQsort(void *base, U32 nelem, U32 width, int (QSORT_CALLBACK *fcmp)(const void *, const void *));

// UNICODE is a windows platform API switching flag. Don't define it on other platforms.
#ifdef UNICODE
#define dT(s)    L##s
#else
#define dT(s)    s
#endif

#define dStrdup(x) dStrdup_r(x, __FILE__, __LINE__)

extern char* dStrdup_r(const char *src, const char *fileName, dsize_t lineNumber);

#endif  // _STRING_FUNCTIONS_H_
