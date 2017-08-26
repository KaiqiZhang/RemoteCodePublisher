#pragma once
/////////////////////////////////////////////////////////////////////
//  CodePublisher.h - publish code to html files                   //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code publisher                  //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a Publisher class to publish source code files 
as web pages.
The web pages contains embeded child links. Each link refers to a 
code file that the displayed file depends on.

Public Interface:
=================
Publisher publisher;                  // create a code publisher instance
publisher.doPublish();                // do publish codes

Build Process:
==============
Required files
- CodePublisher.h, CodePublisher.cpp
- ScopeTable.h, ScopeTable.cpp
- DepAnal.h, DepAnal.cpp
- AbstrSynTree.h, AbstrSynTree.cpp

Maintenance History:
====================
ver 1.0 : 08 Apr 2017
- first release

*/

#include <string>
#include "../Analyzer/DepAnal.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"
#include "ScopeTable.h"

namespace CodePublisher
{
  using namespace CodeAnalysis;

  ///////////////////////////////////////////////////////////////////
  // Publisher class to perform code publishing tasks

  class Publisher
  {
  public:
    using Path = std::string;
    using File = std::string;

    Publisher(DepTable& depTable, Path analysisPath, Path publishPath);
    void doPublish();
    DepTable& depTable() { return _depTable; }

  private:
    AbstrSynTree& ASTref_;
    DepTable _depTable;
    Path _analPath;
    Path _pubPath;
    ScopeTable _scopeTable;

    void genCssFile();
    void genJsFile();
    void genCodePages();
    void genIndexPage();

    void genPrologue(File file, std::ostream& out);
    void genHeader(File file, std::ostream& out);
    void genFooter(std::ostream& out);
    void genDepList(File parent, DepTable::Deps deps, std::ostream& out);
    void genCodeDiv(File file, std::istream& in, std::ostream& out);

    void addCodeLinePrefix(File file, std::vector<std::string>& lines);
    void StringReplace(std::string &strBase, std::string strSrc, std::string strDes);
    void DFS4Scope(ASTNode* pNode);
  };
}
