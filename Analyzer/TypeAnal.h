#pragma once
/////////////////////////////////////////////////////////////////////
//  TypeAnal.h - analyze types in source code files                //
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
This package defines a TypeAnal class to find all types and global 
functions defined in each of a collection of C++ source files, including:
- type definitions: classes, structs, enums, typedefs, and aliases.
- global function definitions
- global data definitions
This class analyze types for an abstract syntax tree.

Public Interface:
=================
TypeAnal typeAnal;                  // create a TypeAnal instance
typeAnal.doTypeAnal();              // do type analysis process

Build Process:
==============
Required files
- TypeAnal.h, TypeAnal.cpp
- TypeTable.h, TypeTable.cpp
- ActionsAndRules.h, ActionsAndRules.cpp
- Parser.h, Parser.cpp
- ScopeStack.h, ScopeStack.cpp
- ConfigureParser.cpp, ItokCollection.h
- SemiExpression.h, SemiExpression.cpp
- Tokenizer.h, Tokenizer.cpp

Maintenance History:
====================
ver 1.1 : 30 Mar 2017
- modified to save relative path
ver 1.0 : 13 Mar 2017
- first release

*/

#include <string>
#include "../TypeTable/TypeTable.h"
#include "../Parser/ActionsAndRules.h"

namespace CodeAnalysis
{
  ///////////////////////////////////////////////////////////////////
  // TypeAnal class do type analysis from a Abstract Syntax Tree

  class TypeAnal
  {
  public:
    using Path = std::string;

    TypeAnal(Path analysisPath);
    void doTypeAnal();

  private:
    void DFSforGlobal(ASTNode* pNode);

    AbstrSynTree& ASTref_;
    TypeTable& TTref_;
    Path path_;
  };

  //----< TypeAnal constructor, initialize private data >------------

  inline TypeAnal::TypeAnal(Path analysisPath) :
    ASTref_(Repository::getInstance()->AST()),
    TTref_(Repository::getInstance()->getTypeTable()),
    path_(analysisPath)
  {
  }
}