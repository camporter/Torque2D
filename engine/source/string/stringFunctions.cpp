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

#include "string/stringFunctions.h"

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#include "console/console.h"

#include "platform/platform_ScriptBinding.h"
#include "string/stringFunctions_ScriptBinding.h"

// concatenates a list of src's onto the end of dst
// the list of src's MUST be terminated by a NULL parameter
// dStrcatl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcatl(char *dst, dsize_t dstSize, ...)
{
   const char* src;
   char *p = dst;

   AssertFatal(dstSize > 0, "dStrcatl: destination size is set zero");
   dstSize--;  // leave room for string termination

   // find end of dst
   while (dstSize && *p)
   {
       p++;
      dstSize--;
   }

   va_list args;
   va_start(args, dstSize);

   // concatenate each src to end of dst
   while ( (src = va_arg(args, const char*)) != NULL )
   {
      while( dstSize && *src )
      {
         *p++ = *src++;
         dstSize--;
      }
   }

   va_end(args);

   // make sure the string is terminated
   *p = 0;

   return dst;
}


int dStrcmp(const UTF16 *str1, const UTF16 *str2)
{
#if defined(TORQUE_OS_WIN32)
   return wcscmp(str1, str2);
#else
   int ret;
   const UTF16 *a, *b;
   a = str1;
   b = str2;

   while ( ((ret = *a - *b) == 0) && *a && *b )
      a++, b++;

   return ret;
#endif
}

int dStricmp(const char *str1, const char *str2)
{
   return strcasecmp(str1, str2);   
}

int dStrncmp(const char *str1, const char *str2, dsize_t len)
{
   return strncmp(str1, str2, len);   
}

int dStrnicmp(const char *str1, const char *str2, dsize_t len)
{
   return strncasecmp(str1, str2, len);   
}

char* dStrcpy(char *dst, const char *src)
{
   return strcpy(dst, src);
}

// copy a list of src's into dst
// the list of src's MUST be terminated by a NULL parameter
// dStrccpyl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcpyl(char *dst, dsize_t dstSize, ...)
{
   const char* src;
   char *p = dst;

   AssertFatal(dstSize > 0, "dStrcpyl: destination size is set zero");
   dstSize--;  // leave room for string termination

   va_list args;
   va_start(args, dstSize);

   // concatenate each src to end of dst
   while ( (src = va_arg(args, const char*)) != NULL )
   {
      while ( dstSize && *src )
      {
         *p++ = *src++;
         dstSize--;
      }
   }

   va_end(args);

   // make sure the string is terminated
   *p = 0;

   return dst;
}

char* dStrncpy(char *dst, const char *src, dsize_t len)
{
   return strncpy(dst, src, len);
}

char* dStrupr(char *str)
{
#if defined(TORQUE_OS_WIN32)

   return _strupr(str);

#else

   if (str == NULL)
      return(NULL);

   char* saveStr = str;
   while (*str)
   {
      *str = toupper(*str);
      str++;
   }
   return saveStr;

#endif
}

char* dStrlwr(char *str)
{
#if defined(TORQUE_OS_WIN32)
   return _strlwr(str);
#else
   if (str == NULL)
      return(NULL);

   char* saveStr = str;
   while (*str)
   {
      *str = tolower(*str);
      str++;
   }
   return saveStr;
#endif
}

char* dStrstr(char *str1, char *str2)
{
   return strstr(str1,str2);
}

char* dStrstr(const char *str1, const char *str2)
{
   return strstr((char *)str1, str2);
}

char* dStristr( char* str1, const char* str2 )
{
   if( !str1 || !str2 )
      return NULL;

   U32 str2len = strlen( str2 );
   while( *str1 )
   {
      if( strncasecmp( str1, str2, str2len ) == 0 )
         return str1;

      ++str1;
   }

   return NULL;
}

const char* dStristr( const char* str1, const char* str2 )
{
   return dStristr( const_cast< char* >( str1 ), str2 );
}

int dStrrev(char* str)
{
  int l = dStrlen(str) - 1; //get the string length
  for (int x = 0; x < l; x++, l--)
  {
    str[x]^=str[l];  //triple XOR Trick
    str[l]^=str[x];  //for not using a temp
    str[x]^=str[l];
  }
  return l;
}

bool dAtob(const char *str)
{
   return !dStricmp(str, "true") || dAtof(str);
   // return !dStricmp(str, "true") || !dStricmp(str, "1") || (0 != dAtoi(str));
}

int dItoa(int n, char s[])
{
   int i, sign;

   // Record sign
   if ((sign = n) < 0)
   {
      // Make n positive
      n = -n;
   }

   i = 0;

   // Generate digits in reverse order
   do {
   	  // Get next digit
      s[i++] = n % 10 + '0';
      // Delete it
   } while ((n /= 10) > 0);

   if (sign < 0)
   {
      s[i++] = '-';
   }

   s[i] = '\0';
   dStrrev(s);
   return dStrlen(s);
}

int dSscanf(const char *buffer, const char *format, ...)
{

   va_list args;
   va_start(args, format);

#if defined(TORQUE_OS_WIN32)

   // Boy is this lame.  We have to scan through the format string, and find out how many
   //  arguments there are.  We'll store them off as void*, and pass them to the sscanf
   //  function through specialized calls.  We're going to have to put a cap on the number of args that
   //  can be passed, 20 for the moment.  Sigh.
   void* sVarArgs[20];
   U32 numArgs = 0;

   for (const char* search = format; *search != '\0'; search++) {
      if (search[0] == '%' && search[1] != '%')
         numArgs++;
   }
   AssertFatal(numArgs <= 20, "Error, too many arguments to lame implementation of dSscanf.  Fix implmentation");

   // Ok, we have the number of arguments...
   for (U32 i = 0; i < numArgs; i++)
      sVarArgs[i] = va_arg(args, void*);
   va_end(args);

   switch (numArgs) {
     case 0: return 0;
     case 1:  return sscanf(buffer, format, sVarArgs[0]);
     case 2:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1]);
     case 3:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2]);
     case 4:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3]);
     case 5:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4]);
     case 6:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5]);
     case 7:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6]);
     case 8:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7]);
     case 9:  return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8]);
     case 10: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9]);
     case 11: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10]);
     case 12: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11]);
     case 13: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12]);
     case 14: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13]);
     case 15: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14]);
     case 16: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15]);
     case 17: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16]);
     case 18: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17]);
     case 19: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17], sVarArgs[18]);
     case 20: return sscanf(buffer, format, sVarArgs[0], sVarArgs[1], sVarArgs[2], sVarArgs[3], sVarArgs[4], sVarArgs[5], sVarArgs[6], sVarArgs[7], sVarArgs[8], sVarArgs[9], sVarArgs[10], sVarArgs[11], sVarArgs[12], sVarArgs[13], sVarArgs[14], sVarArgs[15], sVarArgs[16], sVarArgs[17], sVarArgs[18], sVarArgs[19]);
   }
   return 0;

#else

   return vsscanf(buffer, format, args);

#endif
}

void dPrintf(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
}

int dVprintf(const char *format, va_list arglist)
{
   return vprintf(format, arglist);
}

int dSprintf(char *buffer, dsize_t bufferSize, const char *format, ...)
{
   va_list args;
   va_start(args, format);

   int len = vsnprintf(buffer, bufferSize, format, args);

   va_end(args);

   return len;
}

int dVsprintf(char *buffer, dsize_t bufferSize, const char *format, va_list arglist)
{
   return vsnprintf(buffer, bufferSize, format, arglist);
}

void dQsort(void *base, U32 nelem, U32 width, S32 (QSORT_CALLBACK *fcmp)(const void *, const void *))
{
    if ( nelem == 0 )
        return;

   qsort(base, nelem, width, fcmp);
}

char *dStrdup_r(const char *src, const char *fileName, dsize_t lineNumber)
{
   dsize_t bufferSize = dStrlen(src) + 1;
   char *buffer = (char *) dMalloc_r(bufferSize, fileName, lineNumber);
   dStrncpy(buffer, src, bufferSize);
   return buffer;
}
