/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* parsestr.cpp                                                         */
/* -------------------------------------------------------------------- */

#include "parsestr.h"

/* ----- Constructors/Destructors ------------------------------------- */

ParseString::ParseString(): StrPP()
{
  offset = 0;
}

ParseString::ParseString(const char* p): StrPP(p)
{
  offset = 0;
}

ParseString::ParseString(const StrPP& s): StrPP(s)
{
  offset = 0;
}

ParseString::~ParseString(void)
{
  Reset();
}

/* ----- SetStr/AddStr ------------------------------------------------ */

void ParseString::SetStr(const char* p)
{
  Reset();				// reset the string
  StrPP::SetStr(p);
}

void ParseString::SetStr(const StrPP& s)
{
  Reset();				// reset the string
  StrPP::SetStr(s);
}

void ParseString::AddStr(const char c)
{
  unsigned n = offset;			// save the offset
  Reset();				// reset the string
  StrPP::AddStr(c);			// add new contents

  operator+=(n);			// reapply the offset
}

void ParseString::AddStr(const char* p)
{
  unsigned n = offset;			// save the offset
  Reset();				// reset the string
  StrPP::AddStr(p);			// add new contents

  operator+=(n);			// reapply the offset
}

void ParseString::AddStr(const StrPP& s)
{
  unsigned n = offset;			// save the offset
  Reset();				// reset the string
  StrPP::AddStr(s);			// add new contents

  operator+=(n);			// reapply the offset
}

void ParseString::Reset(void)
{
  strPtr -= offset;			// reset strPtr to zero position
  strLen += offset;			// reset strLen
  offset = 0;
}

const char* ParseString::Offset(int n)
{
  if(strLen >= n-offset)
  {
    strPtr -= offset;			// reset strPtr to zero position
    strLen += offset;			// reset strLen
    offset  = n;			// new absolute offset
    strPtr += n;			// increment strPtr
    strLen -= n;			// set strLen
  }
  return strPtr;
}

/* ----- Operators ---------------------------------------------------- */

StrPP& ParseString::operator=(const char c)
{
  SetStr(c);
  return *this;
}

StrPP& ParseString::operator=(const char* p)
{
  SetStr(p);
  return *this;
}

StrPP& ParseString::operator=(const StrPP& s)
{
  SetStr(s);
  return *this;
}

const char* ParseString::operator++()
{
  if(strLen > 0)
  {
    ++offset;
    ++strPtr;
    --strLen;
  }
  return strPtr;
}

const char* ParseString::operator++(int)
{
  if(strLen > 0)
  {
    offset++;
    strPtr++;
    strLen--;
  }
  return strPtr;
}

const char* ParseString::operator--()
{
  if(offset > 0)
  {
    --offset;
    --strPtr;
    ++strLen;
  }
  return strPtr;
}

const char* ParseString::operator--(int)
{
  if(offset > 0)
  {
    offset--;
    strPtr--;
    strLen++;
  }
  return strPtr;
}

const char* ParseString::operator+=(const int n)
{
  if(strLen >= n)
  {
    offset += n;
    strPtr += n;
    strLen -= n;
  }
  return strPtr;
}

const char* ParseString::operator-=(const int n)
{
  if(offset >= n)
  {
    offset -= n;
    strPtr -= n;
    strLen += n;
  }
  return strPtr;
}
