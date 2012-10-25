/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* Derived from code Copyright 1988, Jim Mischel.  All rights reserved. */
/*                                                                      */
/* regexp.h                                                             */
/* -------------------------------------------------------------------- */

#ifndef _REGEXP_H
#define _REGEXP_H

#include "parsestr.h"

extern int RSTART;
extern int RLENGTH;
extern int NF;

class RegExp
{
private:
  ParseString reExpression;		// holds the regular expression
  StrPP       rePattern;		// holds the compiled version
  char* reLastChar;			// used in Match... functions

  const char* MakePattern(void);	// entry point to compile pattern
  const char* ParseExpression(StrPP&);
  const char* ParseTerm(StrPP&);
  const char* ParseFactor(StrPP&);
  char        ParseEscape(void);
  int         ParseClosure(StrPP&);
  const char* ParseCCL(StrPP&);
  const char* ParseDASH(StrPP&, char);

  int         IsFactor(void);
  const char* SkipTerm(char* pattern);

  int Match(const char*, const char*);	// entry point to match expression
  int MatchTerm(int, char*, char*);
  int MatchOR(int, char*, char*);
  int Match_0_1(int, char*, char*);
  int MatchClosure(int, char*, char*, char*);
  int MatchCCL(char, char*);

public:
  RegExp(void) {}
  RegExp(const StrPP&);

  operator const char*()   { return reExpression(); }
  const char* operator()() { return reExpression(); }
  void operator=(const char*);
  void operator=(const StrPP&);

  friend const char* SetFS(const StrPP& s);
  friend int match(const StrPP&, RegExp&);
  friend int operator==(const StrPP& s, RegExp& re);
  friend int operator!=(const StrPP& s, RegExp& re);
  friend ostream& operator<<(ostream&, const RegExp&);
};

typedef RegExp Regexp;			// allow the use of Regexp
typedef RegExp Regex;			// allow the use of Regex
extern RegExp FS;

inline int operator==(const StrPP& s, RegExp& re)
{
  return (re.Match(s, re.rePattern) == -1) ? 0 : 1;
}

inline int operator!=(const StrPP& s, RegExp& re)
{
  return (re.Match(s, re.rePattern) == -1) ? 1 : 0;
}

inline int match(const StrPP& s, RegExp& re)
{
  return re.Match(s, re.rePattern);
}

int sub(const RegExp& from, const StrPP& to, StrPP& str);
int gsub(const RegExp& from, const StrPP& to, StrPP& str, int count=32767);
int split(const StrPP& s, StrPP*& a, const RegExp& fs);
int index(const StrPP& s, const RegExp& t);

#endif
