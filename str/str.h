/* -------------------------------------------------------------------- */
/* String++ Version 3.10				       04/13/94 */
/*									*/
/* Enhanced string class for Turbo C++/Borland C++.			*/
/* Copyright 1991-1994 by Carl W. Moreland				*/
/*									*/
/* str.h								*/
/* -------------------------------------------------------------------- */

#ifndef _STR_H
#define _STR_H

#include <ctype.h>
#include <string.h>
#include <iostream.h>

class StrPP;
class RegExp;

#if defined toupper
    #undef  toupper
#endif
#if defined tolower
    #undef  tolower
#endif

StrPP toupper(const StrPP& s);
StrPP tolower(const StrPP& s);

typedef StrPP String;			// Not compatible with BC++ 3.x
//typedef StrPP string; 		// Not compatible with BC++ 4.x

class StrPP
{
protected:
  int	strLen; 			// length of the string
  char* strPtr; 			// pointer to the string contents
  int	bufferLen;			// length of the buffer

public:
  StrPP();				// default constructor;
  StrPP(const char c,			// initialize with a character,
	const int n = 1);		//   optional # of characters
  StrPP(const char* p); 		// initialize with a char *
  StrPP(const char* p,			// initialize with a char *,
	const int pos,			//   optional starting char
	const int len = 32767); 	//   optional # of chars
  StrPP(const StrPP& s);		// initialize with another string
  StrPP(const StrPP& s, 		// initialize with another string,
	const int pos,			//   optional starting char
	const int len = 32767); 	//   optional # of chars
  StrPP(const int n);			// initialize with an integer
  StrPP(const unsigned int n);		// initialize with an unsigned integer
  StrPP(const long n);			// initialize with a long int
  StrPP(const unsigned long n); 	// initialize with a unsigned long int
  StrPP(const float n,			// initialize with a float
	const char* format = "");       //   and a format specifier
  StrPP(const double n, 		// initialize with a double
	const char* format = "");       //   and a format specifier

 ~StrPP(void);

protected:
  static int strMinLength;		// minimum memory allocated
  static int strIncLength;		// incremental memory allocated
  static int (*strCompare)(const char*, const char*);
  static int (*strToUpper)(int c);
  static int (*strToLower)(int c);
  static StrPP* findIn; 		// these are for the Find... functions
  static StrPP& findStr;
  static int	findPos;
  static StrPP	fpFormat;		// format string for floats

  virtual void Init() {
    strPtr = 0;
    strLen = 0;
    bufferLen = 0;
  }
  virtual void SetStr(const char c, const int n = 1);
  virtual void SetStr(const char* p);
  virtual void SetStr(const char* p,   const int pos, const int len = 32767);
  virtual void SetStr(const StrPP& s);
  virtual void SetStr(const StrPP& s, const int pos, const int len = 32767);
  virtual void AddStr(const char c);
  virtual void AddStr(const char* p);
  virtual void AddStr(const StrPP& s);
  virtual int  GetSize(int n);
  virtual void ltos(const long n);
  virtual void ultos(const unsigned long n);
  virtual void dtos(const double n, const char* format);

public:
  enum StrModes { LEFT	 = 0,
		  CENTER = 1,
		  RIGHT  = 2,
		  NOCLIP = 0,
		  NOTRIM = 0,
		  CLIP	 = 1,
		  TRIM	 = 2,
		  WHITESPACE = 0 };

  static void SetMinLength(int len = 16) { strMinLength = len; }
  static void SetIncLength(int len = 8)  { strIncLength = len; }
  static void SetCompare(int (*fp)(const char*, const char*) = strcmp) {
    strCompare = fp;
  }
  static void SetToUpper(int (*fp)(int c) = ::toupper) {
    strToUpper = fp;
  }
  static void SetToLower(int (*fp)(int c) = ::tolower) {
    strToLower = fp;
  }
  static void SetCaseSensitivity(int cs = 1);
  static void SetFloatFormat(const char*);
  int  SetSize(int len);		// set/reset bufferLen
  void Minimize(void);			// minimize bufferLen

  virtual operator const char() const	 { return strPtr[0]; }
  virtual operator const char*() const	 { return strPtr; }
  virtual const char  operator *() const { return strPtr[0]; }
  virtual const char* operator()() const { return strPtr; }
  virtual const char* operator()(int pos) const { return strPtr+pos; }
  virtual StrPP       operator()(int pos, int len) const;
  virtual char* ptr(void) const 	 { return strPtr; }
  virtual char* ptr(int pos) const	 { return strPtr+pos; }

  virtual int Length(void) const { return strLen; }
  virtual int Len(void)    const { return strLen; }
  virtual StrPP& toUpper(void); 		// convert to uppercase
  virtual StrPP& toLower(void); 		// convert to lowercase
  virtual int&		 Value(int& n) const;		// int value of str
  virtual unsigned int&  Value(unsigned int& n) const;	// unsigned value of str
  virtual long& 	 Value(long& n) const;		// long int value of str
  virtual unsigned long& Value(unsigned long& n) const; // unsigned long value
  virtual float&	 Value(float& n) const; 	// float value of str
  virtual double&	 Value(double& n) const;	// double value of str

  StrPP&  Left(int len);			// left   len chars
  StrPP&  Right(int len);			// right  len chars
  StrPP&  Mid(int pos, int len);		// middle len chars from pos

  StrPP&  Justify(char type, int len,		// justify string
		  char mode = CLIP|TRIM);
  StrPP&  Trim(int mode = CENTER,		// delete whitespace
	       char ch = WHITESPACE);

  StrPP&  Insert(int pos, const StrPP& s);	// insert substring
  StrPP&  Delete(int pos, int len = 1); 	// delete substring

  StrPP&  Replace(int pos, int len,		// substitute pos+len -> to
		  const StrPP& to);
  int	  Replace(const StrPP& from,		// substitute from -> to
		  const StrPP& to,
		  int count = 32767);
  int	  Replace(const RegExp& from,		// substitute from -> to
		  const StrPP& to,
		  int count = 32767);
  char*   Copy(char*&) const;			// copy string to char*

  int	  Index(const StrPP& t) const;		// position of t in string
  int	  Index(const RegExp& t) const; 	// position of t in string
  StrPP   SubStr(int pos,			// substring at position pos
		 int len = 32767) const;
  int	  Split(StrPP*& a,			// split into an array a on
		const StrPP& fs) const; 	//   field separator fs
  int	  Split(StrPP*& a,			// split into an array a on
		const RegExp& fs) const;	//   field separator fs
  int	  Sub(const StrPP& from,		// substitute from -> to
	      const StrPP& to,
	      int count = 32767);
  int	  Sub(const RegExp& from,		// substitute from -> to
	      const StrPP& to,
	      int count = 32767);

  int	  FindFirst(const StrPP& s) const;	// first occurance of s
  int	  FindNext (void) const;		// next occurance of s
  int	  FindPrev (void) const;		// previous occurance of s
  int	  FindLast (const StrPP& s) const;	// last occurance of s

  virtual StrPP& operator=(const char c);	// str1 = char
  virtual StrPP& operator=(const char* p);	// str1 = char*
  virtual StrPP& operator=(const StrPP& s);	// str1 = string
  virtual StrPP& operator=(const int n);	// str1 = int
  virtual StrPP& operator=(const unsigned int n);  // str1 = uint
  virtual StrPP& operator=(const long n);	// str1 = long
  virtual StrPP& operator=(const unsigned long n); // str1 = ulong
  virtual StrPP& operator=(const float n);	// str1 = float
  virtual StrPP& operator=(const double n);	// str1 = double
  StrPP&  operator+=(const char c);		// str1 += char
  StrPP&  operator+=(const char* p);		// str1 += char*
  StrPP&  operator+=(const StrPP& s);		// str1 += str
  StrPP&  operator*=(const int n);		// str1 *= n
  char&   operator[](const int i) const;	// ch = str[i] or str[i] = ch

  friend StrPP operator+(const StrPP& s1, const StrPP& s2);
  friend StrPP operator+(const StrPP& s,  const char* p);
  friend StrPP operator+(const char* p,   const StrPP& s);
  friend StrPP operator*(const StrPP& s,  const int n);
  friend StrPP operator*(const int n,	  const StrPP& s);

  virtual StrPP& operator<<(const char c);		// s << char
  virtual StrPP& operator<<(const char* p);		// s << char*
  virtual StrPP& operator<<(const StrPP& s);		// s << string
  virtual StrPP& operator<<(const int n);		// s << int
  virtual StrPP& operator<<(const unsigned int n);	// s << uint
  virtual StrPP& operator<<(const long n);		// s << long
  virtual StrPP& operator<<(const unsigned long n);	// s << ulong
  virtual StrPP& operator<<(const float n);		// s << float
  virtual StrPP& operator<<(const double n);		// s << double

  friend int operator==(const StrPP& s1, const StrPP& s2);
  friend int operator!=(const StrPP& s1, const StrPP& s2);
  friend int operator< (const StrPP& s1, const StrPP& s2);
  friend int operator> (const StrPP& s1, const StrPP& s2);
  friend int operator<=(const StrPP& s1, const StrPP& s2);
  friend int operator>=(const StrPP& s1, const StrPP& s2);
  friend int operator==(const StrPP& s,  const char* p);
  friend int operator!=(const StrPP& s,  const char* p);
  friend int operator< (const StrPP& s,  const char* p);
  friend int operator> (const StrPP& s,  const char* p);
  friend int operator<=(const StrPP& s,  const char* p);
  friend int operator>=(const StrPP& s,  const char* p);
  friend int operator==(const char* p,	 const StrPP& s);
  friend int operator!=(const char* p,	 const StrPP& s);
  friend int operator< (const char* p,	 const StrPP& s);
  friend int operator> (const char* p,	 const StrPP& s);
  friend int operator<=(const char* p,	 const StrPP& s);
  friend int operator>=(const char* p,	 const StrPP& s);
};

ostream& operator<<(ostream&, const StrPP&);
istream& operator>>(istream&, StrPP&);

/* ----- Awk-style functions ------------------------------------------ */

inline int length(const char* p) {
  return strlen(p);
}
inline int length(const StrPP& s) {
  return s.Len();
}
int    index(const char* s, const char* t);
StrPP substr(const StrPP& s, int pos, int len = 32767);
int    split(const StrPP& s, StrPP*& a, const StrPP& fs);
int	gsub(const StrPP& from, const StrPP& to, StrPP& str, int count = 32767);
inline int sub(const StrPP& from, const StrPP& to, StrPP& str) {
  return gsub(from, to, str, 1);
}

// Regular Expression versions - defined in regexp.cpp

extern int index(const StrPP& s, const RegExp& t);
extern int split(const StrPP& s, StrPP*& a, const RegExp& fs);
extern int  gsub(const RegExp& from, const StrPP& to, StrPP& str, int count);
extern int   sub(const RegExp& from, const StrPP& to, StrPP& str);

/* ----- Other C-style functions -------------------------------------- */

StrPP	 left(const char* p, int len);
StrPP	right(const char* p, int len);
StrPP	  mid(const char* p, int pos, int len);
StrPP justify(const char* p, char type, int len,
	      char mode=StrPP::CLIP|StrPP::TRIM);
StrPP trim(const char* p, int mode = StrPP::CENTER);

/* ----- Inline functions --------------------------------------------- */

inline StrPP& StrPP::operator=(const int n) {
  operator=((long)n);
  return *this;
}

inline StrPP& StrPP::operator=(const unsigned int n) {
  operator=((unsigned long)n);
  return *this;
}

inline StrPP& StrPP::operator=(const float n) {
  operator=((double)n);
  return *this;
}

inline StrPP& StrPP::operator<<(const int n) {
  operator<<((long)n);
  return *this;
}

inline StrPP& StrPP::operator<<(const unsigned int n) {
  operator<<((unsigned long)n);
  return *this;
}

inline StrPP& StrPP::operator<<(const float n) {
  operator<<((double)n);
  return *this;
}

inline int StrPP::Replace(const StrPP& from, const StrPP& to, int count) {
  return gsub(from, to, *this, count);
}

inline int StrPP::Replace(const RegExp& from, const StrPP& to, int count) {
  return gsub(from, to, *this, count);
}

inline int operator==(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) == 0;
}

inline int operator!=(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) != 0;
}

inline int operator<(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) < 0;
}

inline int operator>(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) > 0;
}

inline int operator<=(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) <= 0;
}

inline int operator>=(const StrPP& s1, const StrPP& s2) {
  return StrPP::strCompare(s1, s2) >= 0;
}

inline int operator==(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) == 0;
}

inline int operator!=(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) != 0;
}

inline int operator<(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) < 0;
}

inline int operator>(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) > 0;
}

inline int operator<=(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) <= 0;
}

inline int operator>=(const StrPP& s, const char* p) {
  return StrPP::strCompare(s, p) >= 0;
}

inline int operator==(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) == 0;
}

inline int operator!=(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) != 0;
}

inline int operator<(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) < 0;
}

inline int operator>(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) > 0;
}

inline int operator<=(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) <= 0;
}

inline int operator>=(const char* p, const StrPP& s) {
  return StrPP::strCompare(p, s) >= 0;
}

inline int operator==(const StrPP& s, const char c) {
  return *s == c;
}

inline int operator!=(const StrPP& s, const char c) {
  return *s != c;
}

inline int operator<(const StrPP& s, const char c) {
  return *s < c;
}

inline int operator>(const StrPP& s, const char c) {
  return *s > c;
}

inline int operator<=(const StrPP& s, const char c) {
  return *s <= c;
}

inline int operator>=(const StrPP& s, const char c) {
  return *s >= c;
}

inline int StrPP::Index(const StrPP& t) const {
  return index(*this, t);
}

inline int StrPP::Index(const RegExp& t) const {
  return index(*this, t);
}

inline StrPP StrPP::SubStr(int p, int n) const {
  return substr(*this, p, n);
}

inline int StrPP::Split(StrPP*& a, const StrPP& fs) const {
  return split(*this, a, fs);
}

inline int StrPP::Split(StrPP*& a, const RegExp& fs) const {
  return split(*this, a, fs);
}

inline int StrPP::Sub(const StrPP& from, const StrPP& to, int count) {
  return gsub(from, to, *this, count);
}

inline int StrPP::Sub(const RegExp& from, const StrPP& to, int count) {
  return gsub(from, to, *this, count);
}

extern const StrPP STR_NULL;		// a global NULL string

#endif
