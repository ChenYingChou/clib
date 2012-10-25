/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* filestr.h                                                            */
/* -------------------------------------------------------------------- */
/* Class for decomposing a filespec into its drive, path, name, and     */
/* extension.                                                           */
/* -------------------------------------------------------------------- */

#ifndef _FILESTR_H
#define _FILESTR_H

#include "str.h"

class FileString
{
public:
  StrPP Drive;			// Ex: "C:"
  StrPP Path;			// Ex: "\BC\CLASSLIB\INCLUDE"
  StrPP FileName;		// Ex: "STRNG.H"
  StrPP Name;			// Ex: "STRNG"
  StrPP Ext;			// Ex: "H"

  FileString(void) {};
  FileString(const char*);
  FileString(const StrPP&);
  void operator=(const char*);
  void operator=(const StrPP&);

private:
  virtual void Process(const StrPP&);
};

#endif
