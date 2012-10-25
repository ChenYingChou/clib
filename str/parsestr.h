/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* parsestr.h                                                           */
/* -------------------------------------------------------------------- */
/* String-derived class for parsing routines.                           */
/* -------------------------------------------------------------------- */

#ifndef _PARSESTR_H
#define _PARSESTR_H

#include "str.h"

class ParseString: public StrPP
{
protected:
  unsigned offset;			// offset for parsing

public:
  ParseString();			// default constructor;
  ParseString(const char* p);		// initialize with a char *
  ParseString(const StrPP& s);		// initialize with another String
 ~ParseString(void);

protected:
  virtual void SetStr(const char* p);
  virtual void SetStr(const StrPP& s);
  virtual void SetStr(const char c, const int n = 1) {}
  virtual void SetStr(const char* p,   const int pos, const int len = 32767) {}
  virtual void SetStr(const StrPP& s, const int pos, const int len = 32767) {}
  virtual void AddStr(const char c);
  virtual void AddStr(const char* p);
  virtual void AddStr(const StrPP& s);

public:
  int  Offset(void) { return offset; }	// return the current offset
  const char* Offset(int n);		// set the absolute offset
  void Reset(void);			// set offset = 0

  StrPP& operator=(const char c);	// str1 = char
  StrPP& operator=(const char* p);	// str1 = char*
  StrPP& operator=(const StrPP& s);	// str1 = str
  StrPP& operator=(const int)           { return((StrPP&)STR_NULL); }
  StrPP& operator=(const unsigned int)  { return((StrPP&)STR_NULL); }
  StrPP& operator=(const long)          { return((StrPP&)STR_NULL); }
  StrPP& operator=(const unsigned long) { return((StrPP&)STR_NULL); }
  StrPP& operator=(const float)         { return((StrPP&)STR_NULL); }
  StrPP& operator=(const double)        { return((StrPP&)STR_NULL); }
  const char* operator++();		// ++str - increment strPtr
  const char* operator++(int);		// str++ - increment strPtr
  const char* operator--();		// --str - decrement strPtr
  const char* operator--(int);		// str-- - decrement strPtr
  const char* operator+=(const int);	// str+=n - increment strPtr
  const char* operator-=(const int);	// str-=n - decrement strPtr
};

#endif
