/* -------------------------------------------------------------------- */
/* String++ Version 3.10				       04/13/94 */
/*									*/
/* Enhanced string class for Turbo C++/Borland C++.			*/
/* Copyright 1991-1994 by Carl W. Moreland				*/
/*									*/
/* str.cpp								*/
/* -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include "str.h"

#if defined(__DJGPP__)
    #define ltoa	itoa
    #define ultoa	itoa
#endif

const StrPP STR_NULL = "";

int    StrPP::strMinLength = 16;
int    StrPP::strIncLength = 8;
int  (*StrPP::strCompare)(const char*, const char*) = strcmp;
int  (*StrPP::strToUpper)(int c) = ::toupper;
int  (*StrPP::strToLower)(int c) = ::tolower;
StrPP* StrPP::findIn  = 0;
StrPP& StrPP::findStr = (StrPP&)STR_NULL;
int    StrPP::findPos = 0;
StrPP  StrPP::fpFormat("%0.4f");

/* ----- Constructors/Destructors ------------------------------------- */

StrPP::StrPP()
{
  Init();
}

StrPP::StrPP(const char c, const int n)
{
  Init();
  SetStr(c, n);
}

StrPP::StrPP(const char* p)
{
  Init();
  SetStr(p);
}

StrPP::StrPP(const char* p, const int pos, const int len)
{
  Init();
  SetStr(p, pos, len);
}

StrPP::StrPP(const StrPP& s)
{
  Init();
  SetStr(s);
}

StrPP::StrPP(const StrPP& s, const int pos, const int len)
{
  Init();
  SetStr(s, pos, len);
}

StrPP::StrPP(const int n)
{
  Init();
  ltos((long)n);
}

StrPP::StrPP(const unsigned int n)
{
  Init();
  ultos((unsigned long)n);
}

StrPP::StrPP(const long n)
{
  Init();
  ltos(n);
}

StrPP::StrPP(const unsigned long n)
{
  Init();
  ultos(n);
}

StrPP::StrPP(const float n, const char* format)
{
  Init();
  dtos((double)n, format);
}

StrPP::StrPP(const double n, const char* format)
{
  Init();
  dtos(n, format);
}

StrPP::~StrPP(void)
{
  if(strPtr)
    delete strPtr;
}

/* ----- SetStr/AddStr ------------------------------------------------ */

void StrPP::SetStr(const char c, const int n)
{
  int i;
  strLen = n;

  if(bufferLen > strLen)		// if the buffer is large enough...
  {
    for(i=0; i<strLen; i++)
      strPtr[i] = c;			// copy n chars
  }
  else					// else create a new buffer
  {
    if(strPtr)
      delete strPtr;

    strPtr = new char[GetSize(strLen)]; // allocate the memory

    for(i=0; i<strLen; i++)
      strPtr[i] = c;			// copy n chars
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::SetStr(const char* p)
{
  strLen = strlen(p);

  if(bufferLen > strLen)		// if the buffer is large enough...
    memcpy(strPtr, p, strLen);		// copy the string
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen)];
    memcpy(tmp, p, strLen);		// copy the string

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::SetStr(const char* p, const int pos, const int len)
{
  int p_len = strlen(p);

  if(pos > p_len)			// error check the start pos
    return;

  strLen = p_len - pos;
  if(len < strLen)			// error check the copy length
    strLen = len;

  if(bufferLen > strLen)		// if the buffer is large enough...
    memcpy(strPtr, p+pos, strLen);	// copy the substring
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen)];
    memcpy(tmp, p+pos, strLen); 	// copy the substring

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::SetStr(const StrPP& s)
{
  strLen = s.strLen;

  if(bufferLen > strLen)		// if the buffer is large enough...
    memcpy(strPtr, s.strPtr, strLen);	// copy the string
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen)];
    memcpy(tmp, s.strPtr, strLen);	// copy the string

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::SetStr(const StrPP& s, const int pos, const int len)
{
  if(pos > s.strLen)			// error check the start pos
    return;

  strLen = s.strLen - pos;
  if(len < strLen)			// error check the copy length
    strLen = len;

  if(bufferLen > strLen)		   // if the buffer is large enough...
    memcpy(strPtr, s.strPtr+pos, strLen);  // copy the substring
  else					   // else create a new buffer
  {
    char* tmp = new char[GetSize(strLen)]; // allocate new memory
    memcpy(tmp, s.strPtr+pos, strLen);	   // copy the substring

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::AddStr(const char c)
{
  strLen += 1;

  if(bufferLen > strLen)		// if the buffer is large enough...
    strPtr[strLen-1] = c;		// append character c
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen)];
    memcpy(tmp, strPtr, strLen-1);	// copy the original string
    tmp[strLen-1] = c;			// append character c

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::AddStr(const char* p)
{
  int s_len = strlen(p);

  if(bufferLen > strLen+s_len)		// if the buffer is large enough...
    memcpy(strPtr+strLen, p, s_len);	// append StrPP p
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen+s_len)];
    memcpy(tmp, strPtr, strLen);	// copy the original string
    memcpy(tmp+strLen, p, s_len);	// append string p

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strLen += s_len;
  strPtr[strLen] = 0;			// add NULL termination
}

void StrPP::AddStr(const StrPP& s)
{
  if(bufferLen > strLen + s.strLen)	// if the buffer is large enough...
    memcpy(strPtr+strLen, s.strPtr, s.strLen);	// append string s
  else					// else create a new buffer
  {
    char* tmp = new char[GetSize(strLen+s.strLen)];
    memcpy(tmp, strPtr, strLen);	// copy the original StrPP
    memcpy(tmp+strLen, s.strPtr, s.strLen);	// append StrPP s

    if(strPtr)
      delete strPtr;
    strPtr = tmp;			// now make strPtr = tmp
  }
  strLen += s.strLen;
  strPtr[strLen] = 0;			// add NULL termination
}

// long to string
void StrPP::ltos(const long n)
{
  char num[15];
  ltoa(n, num, 10);

  SetStr(num);
}

// unsigned long to string
void StrPP::ultos(const unsigned long n)
{
  char num[15];
  ultoa(n, num, 10);

  SetStr(num);
}

// double to string
void StrPP::dtos(const double n, const char* format)
{
  char num[40];

  if(format[0] != 0 && fpFormat != format)
    fpFormat = format;			// use the format if given

  sprintf(num, fpFormat, n);

  SetStr(num);
}

// Gets the buffer size using strMinLength & strIncLength
int StrPP::GetSize(int n)
{
  bufferLen = strMinLength;		// start with strMinLength

  if(n > bufferLen)
  {
    while(n > bufferLen)
      bufferLen += strIncLength;	// incrementally add strIncLength
  }
  bufferLen++;				// add one for NULL termination

  return bufferLen;
}

int StrPP::SetSize(int len)
{
  if(len < strLen)
    return 0;

  bufferLen = len + 1;
  char *tmp = new char[bufferLen];

  memcpy(tmp, strPtr, strLen);		// copy the original string
  tmp[strLen] = 0;			// add NULL termination

  if(strPtr)				// now make strPtr = tmp
    delete strPtr;
  strPtr = tmp;

  return bufferLen-1;
}

// Minimize the buffer size to the exact size of the string
void StrPP::Minimize(void)
{
  if(strLen < bufferLen-1)
    SetSize(strLen);
}

void StrPP::SetCaseSensitivity(int cs)
{
  if(cs == 0)
    strCompare = stricmp;		// do case-insensitive compare
  else
    strCompare = strcmp;		// do case-sensitive compare
}

void StrPP::SetFloatFormat(const char* format)
{
  if(format[0] != 0)
    fpFormat = format;
}

/* -------------------------------------------------------------------- */

StrPP& StrPP::Right(int len)
{
  SetStr(strPtr, strLen-len, len);
  return *this;
}

StrPP& StrPP::Left(int len)
{
  SetStr(strPtr, 0, len);
  return *this;
}

StrPP& StrPP::Mid(int pos, int len)
{
  SetStr(strPtr, pos, len);
  return *this;
}

StrPP& StrPP::Justify(char type, int len, char mode)
{
  if(mode&TRIM)
    Trim();				// delete outter whitespace

  if(strLen >= len && !(mode&CLIP))	// check for out-of-bounds
    return *this;

  if(strLen > len && (mode&CLIP))	// check for clipping
  {
    if(type == LEFT)
      Left(len);
    else if(type == CENTER)
      Mid((strLen-len)/2, len);
    else if(type == RIGHT)
      Right(len);

    return *this;			// return clipped string
  }

  if(type == LEFT)
    *this = *this + StrPP(' ', len-strLen);
  else if(type == CENTER)
    *this = StrPP(' ', (len-strLen)/2) + *this +
	    StrPP(' ', len - (len+strLen)/2);
  else if(type == RIGHT)
    *this = StrPP(' ', len-strLen) + *this;

  strLen = strlen(strPtr);
  return *this; 			// return normal string
}

// Convert the string contents to uppercase
StrPP& StrPP::toUpper(void)
{
  for(int i=0; i<strlen(strPtr); i++)
    strPtr[i] = (char)strToUpper(strPtr[i]);
  return *this;
}

// Convert the string contents to lowercase
StrPP& StrPP::toLower(void)
{
  for(int i=0; i<strlen(strPtr); i++)
    strPtr[i] = (char)strToLower(strPtr[i]);
  return *this;
}

// Return the integer numerical value of the string
int& StrPP::Value(int& n) const
{
  n = atoi(strPtr);
  return n;
}

// Return the unsigned integer numerical value of the string
unsigned& StrPP::Value(unsigned& n) const
{
  n = atoi(strPtr);
  return n;
}

// Return the long integer numerical value of the string
long& StrPP::Value(long& n) const
{
  n = atol(strPtr);
  return n;
}

// Return the unsigned long integer numerical value of the string
unsigned long& StrPP::Value(unsigned long& n) const
{
  n = atol(strPtr);
  return n;
}

// Return the float numerical value of the string
float& StrPP::Value(float& n) const
{
  n = (float)atof(strPtr);
  return n;
}

// Return the double numerical value of the string
double& StrPP::Value(double& n) const
{
  n = atof(strPtr);
  return n;
}

// Insert substring 's' at position 'pos'
StrPP& StrPP::Insert(int pos, const StrPP& s)
{
  if(pos > strLen)			// check for out-of-bounds
    return *this;

  if(bufferLen > strLen + s.strLen)	// if the buffer is big enough...
  {
    memmove(strPtr+pos+s.strLen, strPtr+pos, strLen-pos);
    memcpy(strPtr+pos, s.strPtr, s.strLen);
  }
  else					// need to reallocate the buffer
  {
    GetSize(strLen + s.strLen);
    char* tmp = new char[bufferLen];

    memcpy(tmp, strPtr, pos);
    memcpy(tmp+pos, s.strPtr, s.strLen);
    memcpy(tmp+pos+s.strLen, strPtr+pos, strLen-pos);

    delete strPtr;
    strPtr = tmp;
  }
  strLen += s.strLen;			// increase strLen
  strPtr[strLen] = 0;			// add NULL termination

  return *this;
}

// Delete 'len' characters beginning at 'pos'
StrPP& StrPP::Delete(int pos, int len)
{
  if(pos >= strLen)			// check for out-of-bounds
    return *this;
  if(len > strLen - pos)
    len = strLen - pos;
  if(len == 0)
    len = strLen - pos;

  strLen -= len;
  memmove(strPtr+pos, strPtr+pos+len, strLen-pos);
  strPtr[strLen] = 0;			// add a new NULL terminator

  return *this;
}

// Replace 'len' characters beginning at 'pos' with substring 'to'
StrPP& StrPP::Replace(int pos, int len, const StrPP& to)
{
  Delete(pos, len);
  Insert(pos, to);

  return *this;
}

// Copy the string contents to 'p'
char* StrPP::Copy(char*& p) const
{
  p = new char[strLen + 1];		// allocate memory for p
  memcpy(p, strPtr, strLen);

  return p;
}

// Trim leading and/or trailing characters from the string. 'c' specifies
// the character to be trimmed and defaults to WHITESPACE (spaces & tabs).
StrPP& StrPP::Trim(int mode, char c)
{
  int begin = 0;
  int end = strLen-1;

  if(c == WHITESPACE)			// if we're deleting whitespaces...
  {
    if(mode == LEFT || mode == CENTER)	// delete leading whitespace
    {
      while(isspace(strPtr[begin]) && begin <= end)
	begin++;
    }

    if(mode == RIGHT || mode == CENTER) // delete trailing whitespace
    {
      while(isspace(strPtr[end]) && end >= begin)
	end--;
    }
  }
  else					// else a character was specified
  {
    if(mode == LEFT || mode == CENTER)	// delete leading characters
    {
      while(strPtr[begin] == c && begin <= end)
	begin++;
    }

    if(mode == RIGHT || mode == CENTER) // delete trailing characters
    {
      while(strPtr[end] == c && end >= begin)
	end--;
    }
  }

  SetStr(strPtr, begin, end-begin+1);
  return *this;
}

/* ----- Find methods ------------------------------------------------- */

// Find the first occurrence of substring 's' in the string
int StrPP::FindFirst(const StrPP& s) const
{
  findPos = index(strPtr, s.strPtr);	// do the search

  if(findPos >= 0)			// if 's' was found...
  {
    findIn  = (StrPP*)this;		// ...set the global findIn pointer
    findStr = s;			//   and the global findStr string
  }
  else					// 's' was not found...
  {
    findIn  = 0;			// ...so reset the global vars
    findStr = 0;
  }

  return findPos;			// return the position (-1 if not found)
}

// Find the next occurrence of substring 'findStr' in the string
int StrPP::FindNext(void) const
{
  if(findIn != this)			// wrong string, must run FindFirst
    return -1;
  if(findPos >= strLen) 		// out-of-bounds
    findPos = -1;
  if(findPos == -1)			// no more occurrences
  {
    findIn  = 0;
    findStr = 0;
    return findPos;
  }

  int i = index(strPtr+findPos+1, findStr.strPtr);

  if(i == -1)				// findStr not found
  {
    findIn  = 0;
    findStr = 0;
    findPos = -1;
  }
  else
    findPos += i+1;

  return findPos;
}

// Find the previous occurrence of substring 'findStr' in the string
int StrPP::FindPrev(void) const
{
  if(findIn != this)			// wrong string
    return -1;
  if(findPos < 0)			// out-of-bounds
    findPos = -1;
  if(findPos == -1)			// no more occurrences
  {
    findIn  = 0;
    findStr = 0;
    return findPos;
  }

  for(int i = findPos-findStr.strLen; i >= 0; i--)
  {
    if(memcmp(strPtr+i, findStr.strPtr, findStr.strLen) == 0)
    {
      findPos = i;
      return findPos;
    }
  }
  findIn  = 0;				// findStr not found
  findStr = 0;
  findPos = -1;
  return findPos;
}

// Find the last occurrence of substring 's' in the string
int StrPP::FindLast(const StrPP& s) const
{
  for(int i = strLen-s.strLen; i > 0; i--)
  {
    if(memcmp(strPtr+i, s.strPtr, s.strLen) == 0)
    {
      findIn  = (StrPP*)this;
      findStr = s;
      findPos = i;
      return findPos;
    }
  }
  findIn  = 0;				// findStr not found
  findStr = 0;
  findPos = -1;
  return findPos;
}

/* ----- Operators ---------------------------------------------------- */

StrPP StrPP::operator()(int pos, int len) const
{
  StrPP tmp(strPtr, pos, len);
  return tmp;
}

StrPP& StrPP::operator=(const char c)
{
  SetStr(c);
  return *this;
}

StrPP& StrPP::operator=(const char* p)
{
  SetStr(p);
  return *this;
}

StrPP& StrPP::operator=(const StrPP& s)
{
  SetStr(s);
  return *this;
}

StrPP& StrPP::operator=(const long n)
{
  ltos(n);
  return *this;
}

StrPP& StrPP::operator=(const unsigned long n)
{
  ultos(n);
  return *this;
}

StrPP& StrPP::operator=(const double n)
{
  dtos(n, "");
  return *this;
}

StrPP& StrPP::operator+=(const char c)
{
  AddStr(c);
  return *this;
}

StrPP& StrPP::operator+=(const char* p)
{
  AddStr(p);
  return *this;
}

StrPP& StrPP::operator+=(const StrPP& s)
{
  AddStr(s);
  return *this;
}

StrPP operator+(const StrPP& s, const char* p)
{
  StrPP tmp(s);
  tmp.AddStr(p);
  return tmp;
}

StrPP operator+(const char* p, const StrPP& s)
{
  StrPP tmp(p);
  tmp.AddStr(s);
  return tmp;
}

StrPP operator+(const StrPP& s1, const StrPP& s2)
{
  StrPP tmp(s1);
  tmp.AddStr(s2);
  return tmp;
}

StrPP operator*(const StrPP& s1, const int n)
{
  StrPP tmp(s1);

  for(int i=1; i<n; i++)
    tmp += s1;
  return tmp;
}

StrPP operator*(const int n, const StrPP& s1)
{
  StrPP tmp(s1);

  for(int i=1; i<n; i++)
    tmp += s1;
  return tmp;
}

StrPP& StrPP::operator*=(const int n)
{
  int nlen = n*strLen;

  if(bufferLen > nlen)
  {
    for(int i=0; i<n; i++)
      memcpy(strPtr+i*strLen, strPtr, strLen);
    strPtr[nlen] = 0;
  }
  else
  {
    char *tmp = new char[nlen + 1];

    for(int i=0; i<n; i++)
      memcpy(tmp+i*strLen, strPtr, strLen);
	 tmp[nlen] = 0;

    SetStr(tmp);
  }
  return *this;
}

char& StrPP::operator[](const int n) const
{
  if(n > strLen)
    return *(strPtr + strLen);
  return *(strPtr + n);
}

StrPP& StrPP::operator<<(const char c)
{
  AddStr(c);
  return *this;
}

StrPP& StrPP::operator<<(const char* p)
{
  AddStr(p);
  return *this;
}

StrPP& StrPP::operator<<(const StrPP& s)
{
  AddStr(s);
  return *this;
}

StrPP& StrPP::operator<<(const long n)
{
  char num[15];
  ltoa(n, num, 10);
  AddStr(num);

  return *this;
};

StrPP& StrPP::operator<<(const unsigned long n)
{
  char num[15];
  ultoa(n, num, 10);
  AddStr(num);

  return *this;
};

StrPP& StrPP::operator<<(const double n)
{
  StrPP tmp(n);
  AddStr(tmp);
  return *this;
};

/* -------------------------------------------------------------------- */
/* AWK-style functions							*/
/* -------------------------------------------------------------------- */

int index(const char* s, const char* t)
{
  int pos;
  const char *tmp;

  if((tmp = strstr(s, t)) != NULL)
    pos = (int)(tmp-s);
  else
    pos = -1;

  return pos;
}

StrPP substr(const StrPP& s, int pos, int len)
{
  StrPP tmp(s, pos, len);
  return tmp;
}

int split(const StrPP& s, StrPP*& array, const StrPP& fs)
{
  int i=0, j=0, start=0;
  StrPP *tmp;

  while(i < s.Len())			// find the number of substrings
  {
    if(memcmp(s(i), fs(), fs.Len()) == 0)
      j++;
    i++;
  }
  if(j == 0)
    return 0;

  tmp = new StrPP[j+1]; 		// allocate the array of strings
  i = 0;
  j = 0;

  while(i < s.Len())			// fill in the array
  {
    if(memcmp(s(i), fs(), fs.Len()) == 0)
    {
      tmp[j++] = mid(s, start, i-start);
      i += fs.Len();
      start = i;
    }
    else
      i++;
  }
  tmp[j++] = mid(s, start, i-start);

  array = tmp;
  return j;
}

int gsub(const StrPP& from, const StrPP& to, StrPP& s, int count)
{
  int i=0, j=0;

  while(i <= s.Len() - from.Len())
  {
    if(memcmp(s(i), from(), from.Len()) == 0)
    {
      s = left(s, i) + to + right(s, s.Len()-i-from.Len());
      i += to.Len();
      if(++j == count)
	break;
    }
    else
      i++;
  }
  return j;
}

/* -------------------------------------------------------------------- */
/* C-style functions (return copies of strings) 			*/
/* -------------------------------------------------------------------- */

StrPP toupper(const StrPP& s)
{
  StrPP tmp(s);
  tmp.toUpper();
  return tmp;
}

StrPP tolower(const StrPP& s)
{
  StrPP tmp(s);
  tmp.toLower();
  return tmp;
}

StrPP left(const char* p, int len)
{
  StrPP tmp(p, 0, len);
  return tmp;
}

StrPP right(const char* p, int len)
{
  StrPP tmp(p, strlen(p)-len, len);
  return tmp;
}

StrPP mid(const char* p, int pos, int len)
{
  StrPP tmp(p, pos, len);
  return tmp;
}

StrPP justify(const char* p, char type, int len, char mode)
{
  StrPP tmp(p);
  tmp.Justify(type, len, mode);
  return tmp;
}

StrPP trim(const char* p, int mode)
{
  StrPP tmp(p);
  tmp.Trim(mode);
  return tmp;
}

/* ----- Stream I/O --------------------------------------------------- */

ostream& operator<<(ostream& strm, const StrPP& s)
{
  return strm << s();
}

istream& operator>>(istream& strm, StrPP& s)
{
  char p[256];

  strm >> p;
  s = p;

  return strm;
}
