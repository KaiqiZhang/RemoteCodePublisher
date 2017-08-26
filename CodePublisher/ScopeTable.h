#pragma once
/////////////////////////////////////////////////////////////////////
//  ScopeTable.h - a data structure that store scope start & end   //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to support code publisher                  //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a ScopeTable class.
Those provides support for storing scope start and end line numbers 
during source code publishing.

Public Interface:
=================
ScopeTable table;                                  // create a scope talbe instance
table.addStartLine(".\\file1.h", 1);               // record scope start line 1
table.addEndLine(".\\file1.h", 2);                 // record scope end line 2
ScopeType type = table.lineType(".\\file1.h", 1);  // get type of line 1 in file1.h
table.show();                                      // a simplified show of table content

Build Process:
==============
Required files
- ScopeTable.h, ScopeTable.cpp

Maintenance History:
====================
ver 1.0 : 08 Apr 2017
- first release

*/

#include <unordered_map>

namespace CodePublisher
{
  ///////////////////////////////////////////////////////////////////
  // ScopeTable class represents scope type of a line

  class ScopeTable
  {
  public:
    enum ScopeType { none, start, end };

    using File = std::string;
    using ScopeTableOne = std::unordered_map<size_t, ScopeType>;

    void addStartLine(File file, size_t lineNo);
    void addEndLine(File file, size_t lineNo);
    ScopeType lineType(File file, size_t lineNo);
    void show();  // show table content

  private:
    std::unordered_map<File, ScopeTableOne> _store;
  };
}