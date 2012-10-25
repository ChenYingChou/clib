/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* filestr.cpp                                                          */
/* -------------------------------------------------------------------- */

#include "filestr.h"

FileString::FileString(const char* PathName)
{
  StrPP tmp = PathName;
  Process(tmp);
}

FileString::FileString(const StrPP& PathName)
{
  Process(PathName);
}

void FileString::Process(const StrPP& PathName)
{
  int n;

  n = PathName.FindLast("\\");

  Path     = PathName.SubStr(0, n+1);
  FileName = PathName.SubStr(n+1, PathName.Len()-n-1);

  n = FileName.FindLast(".");

  if(n != -1)
  {
    Name = FileName.SubStr(0, n);
    Ext  = FileName.SubStr(n+1, FileName.Len()-n-1);
  }
  else
    Name = FileName;

  if(Path[1] == ':')
  {
    Drive = Path.SubStr(0,2);
    Path.Delete(0,2);
  }
}

void FileString::operator=(const char* PathName)
{
  StrPP tmp = PathName;
  Process(tmp);
}

void FileString::operator=(const StrPP& PathName)
{
  Process(PathName);
}
