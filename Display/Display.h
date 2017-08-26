#pragma once
/////////////////////////////////////////////////////////////////////
//  Display.h -    display type table, dependency relations, and   //
//                 strongly connected components                   //
//  ver 1.1                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a Display class to display type table, 
dependency relations, and strongly connected components.
All member functions are static, which means you don't need to create
an instance.

Public Interface:
=================
Display::showTypeTable(typeTable);           // display type table
Display::showDb(depDb, std::cout, summary);  // display file dependency summary
Display::showDb(depDb, std::cout, detailed); // display file dependency detail

Build Process:
==============
Required files
- Display.h, Display.cpp
- Utilities.h, Utilities.cpp

Maintenance History:
====================
ver 1.1 : 28 Mar 2017
- remove strong compoment display
- modified dependency dispaly
ver 1.0 : 13 Mar 2017
- first release

*/

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "../TypeTable/TypeTable.h"
#include "../Analyzer/DepAnal.h"

namespace CodeAnalysis
{
  ///////////////////////////////////////////////////////////////////
  // Display class do show contents of type table, dependency relations,
  // and strongly connected components

  class Display
  {
  public:
    static void showTypeTable(TypeTable& table, std::ostream& out = std::cout);
    static void showDepTable(DepTable& depTable, std::ostream& out = std::cout);

  private:
    static void showTypeTableHeader(std::ostream& out = std::cout);
    static void showTypeTableElement(TypeElement& elem, std::ostream& out = std::cout);

    static void showDepHeader(std::ostream& out = std::cout);
    static void showDepFile(DepTable::File file, size_t indent, std::ostream& out = std::cout);

    static std::string trunc(std::string in, size_t count) { return in.substr(0, count); }
    static std::string truncl(std::string in, size_t count) { return in.substr(in.length()-count, in.length()); }
  }; 
}