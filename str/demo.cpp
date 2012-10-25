/* -------------------------------------------------------------------- */
/* String++ Version 3.10                                       04/13/94 */
/*                                                                      */
/* Enhanced string class for Turbo C++/Borland C++.                     */
/* Copyright 1991-1994 by Carl W. Moreland                              */
/*                                                                      */
/* demo.cpp                                                             */
/* -------------------------------------------------------------------- */
/* Demonstration of String++ methods.                                   */
/* -------------------------------------------------------------------- */

#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "str.h"
#include "regexp.h"

void pause(void);

void StrIntro()
{
  clrscr();
  _setcursortype(_NOCURSOR);

  String title1 = "String++ Version 3.10";
  String title2 = "Written by Carl W. Moreland";
  String title3 = "Demonstration of methods & operators";
  String title4 = "Hit any key to continue";
  String title5 = "or <Ctrl>C to exit...";

  cout << "\n\n\n\n\n\n\n\n";
  cout << justify(title1, String::CENTER, 78) << "\n";
  cout << justify(title2, String::CENTER, 78) << "\n\n";
  cout << justify(title3, String::CENTER, 78) << "\n\n";
  cout << justify(title4, String::CENTER, 78) << "\n";
  cout << justify(title5, String::CENTER, 78) << "\n";

  pause();
}

void StrConstr()
{
  cout << "Create a String and assign it \"Hello World.\":\n";
  String str1 = "Hello World.";
  cout << "String str1 = \"" << str1 << "\";\n\n";

  cout << "The String contents are returned by the () operator:\n";
  cout << "str1() = " << str1 << "\n\n";

  cout << "Now assign str1 to a second String:\n";
  String str2 = str1;
  cout << "String str2 = str1;\n";
  cout << "str2() = " << str2 << "\n\n";

  cout << "We can also assign numeric values to strings:\n";
  str2 = 1024;
  cout << "str2 = 1024;\n";
  cout << "str2() = " << str2 << "\n\n";

  cout << "Create multiple characters by passing an optional multiplier to\n";
  cout << "  the String constructor:\n\n";
  str2 = "/* " + String('-', 40) + " */";
  cout << "str2 = \"/* \" + String(\'-\', 40) + \" */\";\n";
  cout << "str2() = " << str2() << "\n\n";

  pause();
}

void StrContents()
{
  String str1 = "Hello World.";

  cout << "Placing a number in the () operator returns the substring\n";
  cout << "  of the String starting at that number:\n";
  cout << "str1(6) = " << str1(6) << "\n\n";

  cout << "Placing a second number in the () operator limits the substring\n";
  cout << "  to that many characters:\n";
  cout << "str1(6,2) = " << str1(6,2) << "\n\n";

  cout << "The nth character is returned by the [] operator:\n";
  cout << "str1[6] = " << str1[6] << "\n\n";

  cout << "The [] operator can also be used to replace the nth character:\n";
  str1[6] = 'w';
  cout << "str1[6] = 'w';\n";
  cout << "str1() = " << str1 << "\n\n";

  cout << "Use the left(), mid(), & right() functions to return substrings:\n\n";
  cout << " left(str1, 5)    = " << left(str1, 5)   << "\n";
  cout << "  mid(str1, 3, 2) = " << mid(str1, 3, 2) << "\n";
  cout << "right(str1, 6)    = " << right(str1, 6)  << "\n\n";

  cout << "The length of str1 is given by the Length() member function:\n";
  cout << "str1.Length() = " << str1.Length() << "\n\n";

  pause();
}

void StrAddOp()
{
  cout << "Create a new String with the contents \"only\":\n\n";
  String str1 = "only";
  cout << "String str1 = \"" << str1 << "\";\n\n";

  cout << "Now use the + operator to add to it:\n\n";
  str1 = "This is " + str1 + " a test";
  cout << "str1 = \"This is \" + str1 + \" a test\";\n";
  cout << "str1() = " << str1() << "\n\n";

  cout << "Create a new String and use the += operator to append to it:\n\n";
  String str2 = "Please ";
  cout << "String str2 = \"" << str2 << "\";\n\n";
  str2 += "stand by...";
  cout << "str2 += \"stand by...\";\n";
  cout << "str2() = " << str2() << "\n\n";

  cout << "Use the inserter operator to chain appendages:\n\n";
  String str3;
  str3 << "The value of " << 'x' << " is " << 100;
  cout << "str3 \<< \"The value of \" \<< 'x' \<< \" is \" \<< 100;\n";
  cout << "str3() = " << str3() << "\n\n";

  pause();
}

void StrCompareOp()
{
  String str1 = "Hello world.";

  cout << "The == operator will work for String == String, String == char*,\n";
  cout << "  or for char* == String:\n\n";
  if(str1 == "Hello world.")
    cout << "str1 == \"Hello world.\"\n";
  else
    cout << "str1 != \"Hello world.\"\n";

  if("Hello world." == str1)
    cout << "\"Hello world.\" == str1\n\n";
  else
    cout << "\"Hello world.\" != str1\n\n";

  cout << "By default, comparisons are case sensitive:\n\n";

  if(str1 == "HELLO WORLD.")
    cout << "str1 == \"HELLO WORLD.\"\n\n";
  else
    cout << "str1 != \"HELLO WORLD.\"\n\n";

  cout << "You can control the case sensitivity of the comparison operators\n";
  cout << "  by calling the SetCaseSensitivity() function:\n\n";

  String::SetCaseSensitivity(0);
  cout << "String::SetCaseSensitivity(0);\n\n";

  if(str1 == "HELLO WORLD.")
    cout << "str1 == \"HELLO WORLD.\"\n\n";
  else
    cout << "str1 != \"HELLO WORLD.\"\n\n";

  String::SetCaseSensitivity(1);

  pause();
}

void StrMiscOp()
{
  String str1 = "Hello world.";

  cout << "Let's create a String that's equal to str1*2:\n\n";
  String str2 = str1*2;
  cout << "String str2 = str1*2;\n";
  cout << "str2() = " << str2() << "\n\n";

  cout << "Now multiply str2 by 2:\n\n";
  str2 *= 2;
  cout << "str2 *= 2;\n";
  cout << "str2() = " << str2() << "\n\n";

  pause();
}

void StrNumbers()
{
  cout << "The String constructor can also accept numbers. In the case of\n";
  cout << " floating point numbers, you can also pass a format specifier\n";
  cout << " which is stored and used in future conversions.\n\n";

  String str1 = 1024;
  cout << "str1 = 1024;\n";
  cout << "str1() = " << str1 << "\n";
  str1 = 655360L;
  cout << "str1 = 655360L;\n";
  cout << "str1() = " << str1 << "\n\n";

  str1 = String(123.456);
  cout << "str1 = String(123.456);\n";
  cout << "str1() = " << str1 << "\n";
  str1 = String(3.14159, "%10.4f");
  cout << "str1 = String(3.14159, \"%10.4f\");\n";
  cout << "str1() = " << str1 << "\n";
  str1 = 123.456;
  cout << "str1 = 123.456;\n";
  cout << "str1() = " << str1 << "\n";
  String::SetFloatFormat("%1.4e");
  str1 = String(1.6e-19);
  cout << "String::SetFloatFormat(\"%1.4e\");\n";
  cout << "str1 = String(1.6e-19);\n";
  cout << "str1() = " << str1 << "\n";
  str1 = 123.456;
  cout << "str1 = 123.456;\n";
  cout << "str1() = " << str1 << "\n\n";

  pause();
}

void StrToUpper()
{
  String str1 = "Hello World.";

  cout << "The C-style function toupper() will return the upper case version\n";
  cout << "  of a String without changing the String itself, whereas the\n";
  cout << "  member function toUpper() will convert the String internally:\n\n";

  cout << "str1() = " << str1 << "\n";
  cout << "toupper(str1) = " << toupper(str1) << "\n";
  cout << "str1() = " << str1 << "\n";
  str1.toUpper();
  cout << "str1.toUpper();\n";
  cout << "str1() = " << str1 << "\n\n";

  pause();
}

void StrInsDel()
{
  String str1 = "This is only a test";
  cout << "str1() = " << str1() << "\n\n";

  cout << "The Delete() member function can be used to delete a substring:\n\n";
  str1.Delete(8, 5);
  cout << "str1.Delete(8, 5);\n";
  cout << "str1() = " << str1() << "\n\n";

  cout << "The Insert() member function can be used to insert a substring:\n\n";
  str1.Insert(8, "still ");
  cout << "str1.Insert(8, \"still \");\n";
  cout << "str1() = " << str1() << "\n\n";

  cout << "The Replace() member function can be used to insert a substring\n";
  cout << " either by replacing another substring or by replacing a given\n";
  cout << " length of characters at a position:\n\n";

  str1.Replace("is still", "continues to be");
  cout << "str1.Replace(\"is still\", \"continues to be\");\n";
  cout << "str1() = " << str1() << "\n\n";
  str1.Replace(5, 15, "is no longer");
  cout << "str1.Replace(5, 15, \"no longer\");\n";
  cout << "str1() = " << str1() << "\n";

  pause();
}

void StrJustify()
{
  cout << "The justify() function will expand a String to a total width of n\n";
  cout << "  by adding blanks and justify the non-blank portion:\n\n";

  String str1 = "Hello world.";
  String str1a = "³" + justify(str1, String::LEFT, 20)   + "³";
  String str1b = "³" + justify(str1, String::CENTER, 20) + "³";
  String str1c = "³" + justify(str1, String::RIGHT, 20)  + "³";

  cout << "String str1 = \"" << str1 << "\";\n\n";
  cout << "str1a = \"³\" + justify(str1, String::LEFT, 20)   + \"³\";\n";
  cout << "str1b = \"³\" + justify(str1, String::CENTER, 20) + \"³\";\n";
  cout << "str1c = \"³\" + justify(str1, String::RIGHT, 20)  + \"³\";\n";
  cout << "str1a() = " << str1a() << "\n";
  cout << "str1b() = " << str1b() << "\n";
  cout << "str1c() = " << str1c() << "\n";

  cout << "\nHere's what happens when clipping is used:\n\n";
  str1a = "³" + justify(str1, String::LEFT, 8, String::CLIP)   + "³";
  str1b = "³" + justify(str1, String::CENTER, 8, String::CLIP) + "³";
  str1c = "³" + justify(str1, String::RIGHT, 8, String::CLIP)  + "³";

  cout << "str1a = \"³\" + justify(str1, String::LEFT, 8, String::CLIP)   + \"³\";\n";
  cout << "str1b = \"³\" + justify(str1, String::CENTER, 8, String::CLIP) + \"³\";\n";
  cout << "str1c = \"³\" + justify(str1, String::RIGHT, 8, String::CLIP)  + \"³\";\n";
  cout << "str1a() = " << str1a() << "\n";
  cout << "str1b() = " << str1b() << "\n";
  cout << "str1c() = " << str1c() << "\n";

  pause();
}

void StrTrim()
{
  cout << "The trim() function will remove leading and trailing whitespace:\n\n";
  String str1 = "\t\t  Hello World. \t ";
  cout << "String str1 = \"\\t\\t  Hello World. \\t \";\n\n";

  String str1a = trim(str1, String::LEFT);
  cout << "str1a = trim(str1, String::LEFT);\n";
  cout << "str1a() = ³" << str1a() << "³\n\n";
  String str1b = trim(str1, String::RIGHT);
  cout << "str1b = trim(str1, String::RIGHT);\n";
  cout << "str1b() = ³" << str1b() << "³\n\n";
  String str1c = trim(str1);
  cout << "str1c = trim(str1);\n";
  cout << "str1c() = ³" << str1c() << "³\n\n";

  cout << "You can also specify the character to be trimmed:\n\n";
  str1 = "Here we go again..........";
  cout << "str1 = " << str1 << "\n";
  str1.Trim(String::RIGHT, '.');
  cout << "str1.Trim(String::RIGHT, '.');\n";
  cout << "str1 = " << str1 << "\n";

  pause();
}

void StrFind()
{
  int pos;
  String str1 = "d:\\prog\\turboc\\tutor";

  cout << "FindFirst() returns the location of the first instance of a \n";
  cout << " substring in a String. FindNext() will return the location of\n";
  cout << " subsequent instances. FindLast() finds the last instance, and\n";
  cout << " FindPrev() finds subsequent previous instances.\n\n";

  cout << "String str1 = \"" << str1 << "\";\n\n";

  pos = str1.FindFirst("\\");
  cout << "str1.FindFirst(\"\\\") = " << pos << "\n";

  while((pos = str1.FindNext()) != -1)
    cout << "str1.FindNext() = " << pos << "\n";
  cout << "str1.FindNext() = " << pos << " (No more found)\n";

  pos = str1.FindLast("tu");
  cout << "\nstr1.FindLast(\"tu\") = " << pos << "\n";

  while((pos = str1.FindPrev()) != -1)
    cout << "str1.FindPrev() = " << pos << "\n";
  cout << "str1.FindPrev() = " << pos << " (No more found)\n";

  pause();
}

void Awk()
{
  int i, pos, num;
  String *array;
  String str1 = "HELLO WORLD.";
  String str3 = "This is only a test";
  String str4 = "Please stand by...";

  cout << "The following are AWK functions.\n\n";

  cout << "index() returns the location of a substring in a String:\n\n";
  pos = index(str1, "WOR");
  cout << "str1() = " << str1 << "\n";
  cout << "pos = index(str1, \"WOR\");\n";
  cout << "pos = " << pos << "\n";

  pause();

  cout << "split() will split a String at a given substring delimiter:\n\n";
  String str7 = "d:\\prog\\turboc\\tutor";
  num = split(str7, array, "\\");

  cout << "String str7 = \"d:\\prog\\turboc\\tutor\";\n";
  cout << "num = split(str7, array, \"\\\");\n\n";

  cout << "In this example, str7 is split using the \\ character as the\n";
  cout << "  delimiter. The results are placed in array, which now contains:\n\n";
  for(i=0; i<num; i++)
    cout << "array[" << i << "] = " << array[i] << "\n";

  pause();

  cout << "gsub() performs a global substitution. In this example, we want to\n";
  cout << "  replace all \\'s with /'s:\n\n";
  cout << "str7() = " << str7 << "\n";
  i = gsub("\\", "/", str7);
  cout << "i = gsub(\"\\\", \"/\", str7);\n";
  cout << "str7() = " << str7 << "\n";
  cout << "i = " << i << "\n";

  pause();

  cout << "sub() performs a one-time substitution:\n\n";
  cout << "str1() = " << str1 << "\n";
  i = sub("LO", "P ME,", str1);
  cout << "i = sub(\"LO\", \"P ME,\", str1);\n";
  cout << "str1() = " << str1 << "\n";
  cout << "i = " << i << "\n";

  pause();

  cout << "substr() returns a substring of the String starting at n. If a\n";
  cout << "  a third parameter is given, it represents the maximum number\n";
  cout << "  of characters to return\n\n";

  String str_3 = substr(str3, 8);
  String str_4 = substr(str4, 7, 5);
  cout << "str3() = " << str3 << "\n";
  cout << "substr(str3, 8) = " << str_3 << "\n\n";
  cout << "str4() = " << str4 << "\n";
  cout << "substr(str4, 7, 5) = " << str_4 << "\n\n";

  pause();
}

void RegularExpr()
{
  String str1 = "void *ptr = &var;    // assign the address of var to ptr";

  cout << "Regular expressions can be used to extract comments from a file.\n";
  cout << "Assume the following line of code is read into String str1:\n\n";
  cout << str1 << "\n\n";
  cout << "We want to find the portion that begins with \"/\/\" so create a\n";
  cout << "  regular expression for this:\n\n";

  String str1a;
  Regexp re1 = String("//.*");
  if(match(str1, re1) != -1)
    str1a = substr(str1, RSTART, RLENGTH);

  cout << "Regexp re1 = String(\"" << re1  << "\");\n";
  cout << "if(match(str1, re1) != -1)\n";
  cout << "  str1a = substr(str1, RSTART, RLENGTH);\n\n";
  cout << "str1a() = " << str1a << "\n";

  pause();

  cout << "We can use regular expressions to test for a data type.\n\n";
  Regexp reFloat  = String("^[+-]?([0-9]+[.]?[0-9]*$|[.][0-9]+$)");
  Regexp reExp = String("^[+-]?([0-9]+[.]?[0-9]*|[.][0-9]+)([eE][+-]?[0-9]+)?$");
  cout << "Regexp reFloat = String(\"" << reFloat  << "\");\n";
  cout << "Regexp reExp = String(\"" << reExp << "\");\n\n";

  String str2 = "-1.23";
  String str3 = "+1.6e-19";

  cout << "String str2 = \"" << str2 << "\";\n";
  cout << "String str3 = \"" << str3 << "\";\n\n";

  cout << "if(str2 == reFloat)\n";
  cout << "  cout << \"str2 is type float\\n\";\n";
  cout << "if(str3 == reExp)\n";
  cout << "  cout << \"str3 is exponential notation\\n\";\n\n";

  if(str2 == reFloat)
    cout << "str2 is type float\n";
  if(str3 == reExp)
    cout << "str3 is exponential notation\n";

  pause();
}

main()
{
  char ch;
  int i;
  const int n = 16;
  String s[n];

  s[0]  = "String++ Version 3.1 Main Menu";
  s[1]  = "(a)    Constructors       ";
  s[2]  = "(b)    Contents           ";
  s[3]  = "(c)    Addition           ";
  s[4]  = "(d)    Comparison         ";
  s[5]  = "(e)    Misc operators     ";
  s[6]  = "(f)    Numbers            ";
  s[7]  = "(g)    toupper            ";
  s[8]  = "(h)    Insert/Delete      ";
  s[9]  = "(i)    Justification      ";
  s[10] = "(j)    Trim               ";
  s[11] = "(k)    Find               ";
  s[12] = "(l)    AWK functions      ";
  s[13] = "(m)    Regular Expressions";
  s[14] = "";
  s[15] = "(x)    Exit               ";

  for(i=0; i<n; i++)
    s[i].Justify(String::CENTER, 78, String::NOTRIM);

  StrIntro();

  while(1)
  {
    cout << "\n\n" << s[0] << "\n\n";
    for(i=1; i<n; i++)
      cout << s[i] << "\n";

    while(1)
    {
      ch = tolower(getch());
      if(!ch)
      {
        getch();
        continue;
      }
      switch(ch)
      {
        case 'a': clrscr(); StrConstr();    break;
        case 'b': clrscr(); StrContents();  break;
        case 'c': clrscr(); StrAddOp();     break;
        case 'd': clrscr(); StrCompareOp(); break;
        case 'e': clrscr(); StrMiscOp();    break;
        case 'f': clrscr(); StrNumbers();   break;
        case 'g': clrscr(); StrToUpper();   break;
        case 'h': clrscr(); StrInsDel();    break;
        case 'i': clrscr(); StrJustify();   break;
        case 'j': clrscr(); StrTrim();      break;
        case 'k': clrscr(); StrFind();      break;
        case 'l': clrscr(); Awk();          break;
        case 'm': clrscr(); RegularExpr();  break;
        case 'x': clrscr(); _setcursortype(_NORMALCURSOR); exit(0);
        default: continue;
      }
      break;
    }
  }
}

void pause(void)
{
  char ch = getch();
  if(!ch)
    getch();
  if(ch == 0x03)
  {
    _setcursortype(_NORMALCURSOR);
    exit(0);
  }
  clrscr();
  cout << "\n";
}
