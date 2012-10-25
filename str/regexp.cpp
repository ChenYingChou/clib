/* -------------------------------------------------------------------- */
/* String++ Version 3.10				       04/13/94 */
/*									*/
/* Enhanced string class for Turbo C++/Borland C++.			*/
/* Copyright 1991-1994 by Carl W. Moreland				*/
/*									*/
/* Derived from code Copyright 1988, Jim Mischel.  All rights reserved. */
/*									*/
/* regexp.cpp								*/
/* -------------------------------------------------------------------- */

#ifndef __STDDEF_H
#include <stddef.h>
#endif

#include "regexp.h"

#define _RE_TRUE_	-1
#define _RE_FALSE_	 0
#define _RE_ALMOST_	 1	// returned when a closure matches a NULL

const char ENDSTR	= '\0';
const char EOL		= '$';
const char BOL		= '^';
const char NEGATE	= '^';
const char CCL		= '[';
const char NCCL 	= ']';
const char CCLEND	= ']';
const char ANY		= '.';
const char DASH 	= '-';
const char OR		= '|';
const char ESCAPE	= '\\';
const char LPAREN	= '(';
const char RPAREN	= ')';
const char CLOSURE	= '*';
const char POS_CLO	= '+';
const char ZERO_ONE	= '?';
const char LITCHAR	= 'c';
const char END_TERM	= 'e';
const StrPP FS_DEFAULT	= "[ \t]+";

int    RSTART;			// start of matched substring
int    RLENGTH; 		// length of matched substring
int    NF;			// number of fields from most current split
RegExp FS(FS_DEFAULT);		// global field separator

/* -------------------------------------------------------------------- */

RegExp::RegExp(const StrPP& s)
{
  reExpression = s;

  if(reExpression() != "")
    MakePattern();
}

void RegExp::operator=(const char* p)
{
  reExpression = p;

  if(reExpression() != "")
    MakePattern();
}

void RegExp::operator=(const StrPP& s)
{
  reExpression = s;

  if(reExpression() != "")
    MakePattern();
}

/* -------------------------------------------------------------------- */

// MakePattern() - "compile" the regular expression into rePattern

const char* RegExp::MakePattern(void)
{
  static StrPP tmp;

  if(ParseExpression(tmp) == "" || reExpression != ENDSTR)
  {
    rePattern = "";
    reExpression.Reset();
    return NULL;
  }
  rePattern = tmp;
  reExpression.Reset();
  return rePattern;
}

// ParseExpression() - Parse and translate an expression. Returns a pointer
// to the compiled expression, or NULL on error.

const char* RegExp::ParseExpression(StrPP& expr)
{
  StrPP term;
  expr = "";

  if(ParseTerm(term) == "")             // get the first term
    return(NULL);

  while(reExpression == OR)		// parse all subsequent terms
  {
    expr += OR;
    expr += term;
    expr += END_TERM;
    reExpression++;

    if(ParseTerm(term) == "")
      return (NULL);
  }
  expr += term;
  expr += END_TERM;

  return expr;
}

// ParseTerm() - parse and translate a term.  Returns a pointer to the
// compiled term or NULL on error.

const char* RegExp::ParseTerm(StrPP& term)
{
  StrPP factor;
  term = "";

  if(reExpression == BOL)
  {
    term += reExpression[0];
    reExpression++;
  }
  do
  {
    if(ParseFactor(factor) == "")
      return(NULL);

    term += factor;
  }
  while(IsFactor());			// parse all factors of this term

  return term;
}

// IsFactor() - returns _RE_TRUE_ for a valid factor character

int RegExp::IsFactor(void)
{
  static char* nfac_chars = "^|)]+?*";

  return (strchr(nfac_chars, reExpression[0]) == NULL) ? _RE_TRUE_ : _RE_FALSE_;
}

// ParseFactor() - parse and translate a factor.  Returns a pointer to the
// compiled factor or NULL on error.

const char* RegExp::ParseFactor(StrPP& factor)
{
  StrPP tmp;
  factor = "";

  switch(reExpression[0])
  {
    case LPAREN:			// ( - parenthesised expression
      reExpression++;
      ParseExpression(tmp);
      factor += tmp;
      if(reExpression != RPAREN)
	return(NULL);
      reExpression++;
      break;

    case CCL:				// [ - character class; Ex: [0-9]
      reExpression++;
      ParseCCL(tmp);
      factor += tmp;
      if(reExpression != CCLEND)
	return(NULL);
      reExpression++;
      break;

    case ANY:				// .
    case EOL:				// $
      factor += reExpression[0];
      reExpression++;
      break;

    case ESCAPE :			// \
      reExpression++;
      factor += LITCHAR;
      factor += ParseEscape();
      reExpression++;
      break;

    case CLOSURE:			// *
    case POS_CLO:			// +
    case ZERO_ONE:			// ?
    case NEGATE:			// ^
    case CCLEND:			// ]
    case RPAREN:			// )
    case OR:				// |
      return(NULL);			// not valid characters

    default:				// literal character
      factor += LITCHAR;
      factor += reExpression[0];
      reExpression++;
      break;
  }

  if(reExpression == CLOSURE  ||	// check for closure
     reExpression == ZERO_ONE ||
     reExpression == POS_CLO)
  {
    if(ParseClosure(factor) == _RE_FALSE_)
      return(NULL);
  }
  return factor;
}

// ParseEscape() - returns ASCII value of character(s) following ESCAPE

char RegExp::ParseEscape(void)
{
  static char ch;

  switch(reExpression[0])
  {
    case 'b': reExpression++; return ('\b');    // backspace
    case 't': reExpression++; return ('\t');    // tab
    case 'f': reExpression++; return ('\f');    // formfeed
    case 'n': reExpression++; return ('\n');    // linefeed
    case 'r': reExpression++; return ('\r');    // carriage return

    case '0':                           // 0-7 is octal constant
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    {
		ch = (char)(reExpression[0] - '0');
      reExpression++;
      if(reExpression >= '0' && reExpression < '8')
      {
	ch <<= 3;
		  ch += (char)(reExpression[0] - '0');
		  reExpression++;
		}
		if(reExpression >= '0' && reExpression < '8')
		{
		  ch <<= 3;
		  ch += (char)(reExpression[0] - '0');
		  reExpression++;
		}
		return(ch);
	 }
	 default:				// otherwise, just that char
		return(reExpression[0]);
  }
}

// ParseClosure() - place closure character and size before the factor
// in the compiled string.

int RegExp::ParseClosure(StrPP& factor)
{
  if(factor.Len() > 253)
    return _RE_FALSE_;			// closure expression too large

  factor.Insert(0, "  ");
  factor[0] = reExpression[0];
  factor[1] = (char)(factor.Len()-2);

  reExpression++;

  return _RE_TRUE_;
}

// ParseCCL() - parse and translate a character class. Return pointer to the
// compiled class or NULL on error.

const char* RegExp::ParseCCL(StrPP& ccl)
{
  int first = _RE_TRUE_;
  int len;

  ccl = "[ ";

  if(reExpression == NEGATE)		// if first character is NEGATE (^)
  {
    ccl[0] = NCCL;			// then we have a negated
    reExpression++;			// character class
  }

  // parse all characters up to the closing bracket or end of string marker

  while(reExpression != CCLEND && reExpression != ENDSTR)
  {
    if(reExpression == DASH && first == _RE_FALSE_)	// DASH, check for range
    {
      reExpression++;
      if(reExpression == NCCL)
	ccl += DASH;			// not range, literal DASH
      else
      {
	ParseDASH(ccl, reExpression);
	reExpression++;
      }
    }
    else
    {
      if(reExpression == ESCAPE)
      {
	reExpression++;
	ccl += ParseEscape();
	reExpression++;
      }
      else
      {
	ccl += reExpression[0];
	reExpression++;
      }
    }
    first = _RE_FALSE_;
  }

  len = ccl.Len()-2;

  if(len > 255)
    return NULL;			// character class too large
  else
  {
    ccl[1] = (char)len; 		// store CCL length at ccl[1]
    return ccl;
  }
}

// ParseDASH() - fill in range characters.

const char* RegExp::ParseDASH(StrPP& ccl, char ch)
{
  static char c;

  for(c = (char)(ccl[ccl.Len() - 1] + 1); c <= ch; c++)
    ccl += c;

  return ccl;
}

/* -------------------------------------------------------------------- */

// Match() - Return the position of the first character of the left-most
// longest substring of s that Matches pattern, or NULL if no match is
// found. Sets RSTART and RLENGTH.

int RegExp::Match(const char* s, const char* pattern)
{
  char* c = (char*)s;
  reLastChar = (char*)s;

  while(*c != ENDSTR)
  {
    if(MatchTerm(int(c-(char*)s), c, (char*)pattern) != _RE_FALSE_)
    {
      RSTART  = int(c-(char*)s);
      RLENGTH = int(reLastChar - c);
      return RSTART;
    }
    c++;
  }
  RSTART = RLENGTH = 0;
  return -1;
}

// MatchTerm() - Match a compiled term. Returns _RE_TRUE_, _RE_FALSE_,
// or _RE_ALMOST_.

int RegExp::MatchTerm(int offset, char* s, char* pattern)
{
  reLastChar = s;

  if(*pattern == ENDSTR)
    return _RE_FALSE_;

  do
  {
    switch(*pattern)
    {
      case BOL: 			// ^ - match beginning of line
	if(offset != 0)
	  return _RE_FALSE_;
	pattern++;
	break;

      case LITCHAR:			// c - match literal character
	if(*s++ != *++pattern)
	  return _RE_FALSE_;
	pattern++;
	break;

      case END_TERM:			// e - skip end-of-term character
	pattern++;
	break;

      case ANY: 			// . - match any character
	if(*s++ == ENDSTR)		//     except end of string
	  return _RE_FALSE_;
	pattern++;
	break;

      case OR:
	return MatchOR(offset, s, pattern);

      case CCL: 			// [ - character class
      case NCCL:			// ]
	if(*s == ENDSTR)
	  return _RE_FALSE_;
	if(!MatchCCL(*s++, pattern++))
	  return _RE_FALSE_;
	pattern += pattern[0] + 1;
	break;

      case EOL: 			// $ - match end of string
	if(*s != ENDSTR)
	  return _RE_FALSE_;
	pattern++;
	break;

      case ZERO_ONE:			// ?
	return Match_0_1(offset, s, pattern);

      case CLOSURE:			// *
      case POS_CLO:			// +
      {
	char ClosurePattern[1024];

	strncpy(ClosurePattern, pattern+2, *(pattern+1));
	ClosurePattern[*(pattern+1)] = ENDSTR;
	return MatchClosure(offset, s, pattern, ClosurePattern);
      }

      default:

      // If we get to this point, then something has gone very wrong.
      // Most likely, someone has tried to match with an invalid
      // compiled pattern.

	return _RE_FALSE_;
    }
    reLastChar = s;
  } while(*pattern != ENDSTR);

  return _RE_TRUE_;
}

// MatchOR() - Handles selection processing.

int RegExp::MatchOR(int offset, char* s, char* pattern)
{
  char tmp_pattern[1024];
  char *t1, *t2, *junk;

  // The first case is build into tmp_pattern. Second case is already there.
  // Both patterns are searched to determine the longest matched substring.

  tmp_pattern[0] = ENDSTR;
  pattern++;
  junk = (char*)SkipTerm(pattern);

  strncat(tmp_pattern, pattern, int(junk-pattern));
  strcat(tmp_pattern, SkipTerm(junk));
  t1 = (MatchTerm(offset, s, tmp_pattern) != _RE_FALSE_) ? reLastChar : NULL;

  // The second pattern need not be searched if the first pattern results
  // in a match through to the end of the string, since the longest possible
  // match has already been found.

  if(t1 == NULL || *reLastChar != ENDSTR)
  {
    t2 = (MatchTerm(offset, s, junk) != _RE_FALSE_) ? reLastChar : NULL;

    // determine which matched the longest substring

    if(t1 != NULL && (t2 == NULL || t1 > t2))
      reLastChar = t1;
  }
  return (t1 == NULL && t2 == NULL) ? _RE_FALSE_ : _RE_TRUE_;
}

// SkipTerm() - Skip over the current term and return a pointer to the
// next term in the pattern.

const char* RegExp::SkipTerm(char* pattern)
{
  register int nterm = 1;

  while(nterm > 0)
  {
    switch(*pattern)
    {
      case OR:
	nterm++;
	break;

      case CCL:
      case NCCL:
      case CLOSURE:
      case ZERO_ONE:
      case POS_CLO:
	pattern++;
	pattern += pattern[0];
	break;

      case END_TERM:
	nterm--;
	break;

      case LITCHAR:
	pattern++;
	break;
    }
    pattern++;
  }
  return pattern;
}

// Match_0_1() - Match the ZERO_ONE operator. First, this routine attempts
// to match the entire pattern with the input string. If that fails, it
// skips over the closure pattern and attempts to match the rest of the
// pattern.

int RegExp::Match_0_1(int offset, char* s, char* pattern)
{
  char* old_s = s;

  if(MatchTerm(offset, s, pattern+2) == _RE_TRUE_)
    return _RE_TRUE_;
  else if(MatchTerm(offset, old_s, pattern+2+*(pattern+1)) == _RE_FALSE_)
    return _RE_FALSE_;
  else
    return _RE_ALMOST_;
}

// MatchClosure() - Match CLOSURE and POS_CLO. Match as many of the
// closure patterns as possible, then attempt to match the remaining
// pattern with what's left of the input string. Backtrack until we've
// either matched the remaing pattern or we arrive back at where we
// started.

int RegExp::MatchClosure(int offset, char* s,
			 char* pattern, char* ClosurePattern)
{
  char* old_s = s;

  if(MatchTerm(offset, s, ClosurePattern) == _RE_TRUE_)
  {
    old_s = reLastChar;
    if(MatchClosure(offset, old_s, pattern, ClosurePattern) != _RE_FALSE_)
      return _RE_TRUE_;
    else
      return MatchTerm(offset, old_s, pattern+2+*(pattern+1));
  }
  else if(*pattern != CLOSURE)	  // POS_CLO requires at least one match
    return _RE_FALSE_;
  else if(MatchTerm(offset, old_s, pattern+2+*(pattern+1)) == _RE_TRUE_)
    return _RE_ALMOST_;
  else
    return _RE_FALSE_;
}

// MatchCCL() - Match a character class or negated character class

int RegExp::MatchCCL(char c, char* pattern)
{
  register int x;
  char ccl = *pattern++;

  for(x = *pattern; x > 0; x--)
  {
    if(c == pattern[x])
      return(ccl == CCL);
  }
  return(ccl != CCL);
}

/* -------------------------------------------------------------------- */

// Return the position of the first instance of t in s, or -1 if no match.

int index(const StrPP& s, const RegExp& t)
{
  return match(s, (RegExp&)t);
}

// sub() - Substitute 'to' for the leftmost longest substring of str
// matched by the regular expression 'from'. Return number of substitutions
// made.

int sub(const RegExp& from, const StrPP& to, StrPP& str)
{
  if(match(str, (RegExp&)from) == -1)
    return 0;

  str.Replace(RSTART, RLENGTH, to);
  return 1;
}

// gsub() - Substitute 'to' globally for all substrings in str matched by
// the regular expression 'from'. Return number of substitutions made.

int gsub(const RegExp& from, const StrPP& to, StrPP& str, int count)
{
  int n = 0;
  ParseString tmp = str;

  while(match(tmp, (RegExp&)from) != -1)
  {
    tmp.Replace(RSTART, RLENGTH, to);
    tmp += RSTART+RLENGTH;
    if(++n == count)
      break;
  }
  tmp.Reset();
  str = tmp;

  return n;
}

// split() - Split the string s into fields in the array a on field
// separator fs. Returns number of fields.  Also sets the global variable
// NF.

int split(const StrPP& s, StrPP*& array, const RegExp& fs)
{
  StrPP *tmp;
  ParseString ps = s;
  int rstart  = RSTART; 		// save RSTART and RLENGTH
  int rlength = RLENGTH;
  NF = 1;

  while(match(ps, (RegExp&)fs) != -1)
  {
    ps += RSTART+RLENGTH;
    NF++;
  }

  tmp = new StrPP[NF];
  ps.Reset();
  NF = 0;

  while(match(ps, (RegExp&)fs) != -1)
  {
    tmp[NF++] = ps(0, RSTART);
    ps += RSTART+RLENGTH;
  }
  array = tmp;

  RSTART  = rstart;			// restore globals
  RLENGTH = rlength;
  return (NF);
}

ostream& operator<<(ostream& strm, const RegExp& re)
{
  return strm << re.reExpression();
}

