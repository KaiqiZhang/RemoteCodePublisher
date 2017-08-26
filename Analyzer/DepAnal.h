#pragma once
/////////////////////////////////////////////////////////////////////
//  DepAnal.h - analyze dependency relationships between files     //
//  ver 1.1                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
//  Modified from: Jim Fawcett, CSE687                             //
//                 Object Oriented Design, Spring 2017             //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a TypeAnal class to find, for each file in a
specified file collection, all other files from the file collection on
which they depend.
This class ckeck if a file contains tokens in the type table.

Public Interface:
=================
DepAnal depAnal(fileMap);       // create an instance with filemap
depAnal.doDepAnal();            // do analyze dependencies

Build Process:
==============
Required files
- TypeAnal.h, TypeAnal.cpp
- DepAnal.h, DepAnal.cpp
- Tokenizer.h, Tokenizer.cpp
- ActionsAndRules.h, ActionsAndRules.cpp
- Parser.h, Parser.cpp
- ScopeStack.h, ScopeStack.cpp
- ConfigureParser.cpp, ItokCollection.h
- SemiExpression.h, SemiExpression.cpp
- FileSystem.h, FileSystem.cpp

Maintenance History:
====================
ver 1.1 : 28 Mar 2017
- removed NoSqlDb, use a simple data structure to store dependencies
ver 1.0 : 13 Mar 2017
- first release

*/

#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../Parser/ActionsAndRules.h"

namespace CodeAnalysis
{
  class DepTable
  {
  public:
    using File = std::string;
    using Deps = std::unordered_set<File>;
    using Item = std::pair<File, Deps>;
    using iterator = typename std::unordered_map<File, Deps>::iterator;

    void addFile(File parent);
    void addDepFile(File parent, File child);
    Deps getDepFiles(File parent);

    const size_t size() { return _store.size(); }
    iterator begin() { return _store.begin(); }
    iterator end() { return _store.end(); }

  private:
    std::unordered_map<File, Deps> _store;
  };

  inline void DepTable::addFile(File parent)
  {
    if (_store.find(parent) == _store.end())
    {
      Deps deps;
      _store[parent] = deps;
    }
  }

  inline void DepTable::addDepFile(File parent, File child)
  {
    if (parent == child) return;

    if (_store.find(parent) != _store.end())
    {
      Deps &deps = _store[parent];
      deps.insert(child);
    }
    else
    {
      Deps deps;
      deps.insert(child);
      _store[parent] = deps;
    }
  }

  inline DepTable::Deps DepTable::getDepFiles(File parent)
  {
    if (_store.find(parent) != _store.end())
    {
      return _store[parent];
    }
    else
    {
      Deps deps;
      return deps;
    }
  }

  ///////////////////////////////////////////////////////////////////
  // DepAnal class do dependencies analysis of source code files

  class DepAnal
  {
  public:
    using Pattern = std::string;
    using Path = std::string;
    using File = std::string;
    using Files = std::vector<File>;
    using FileMap = std::unordered_map<Pattern, Files>;

    DepAnal(FileMap& fileMap, Path analysisPath);
    void doDepAnal();
    void initDepTable();
    DepTable& depTable() { return depTable_; }

  private:
    TypeTable& TTref_;
    FileMap& fileMap_;
    DepTable depTable_;
    Path path_;

    void distTypes(std::unordered_set<std::string>& tokens, std::string file);
  };

  //----< DepAnal constructor, initialize private data >-------------

  inline DepAnal::DepAnal(FileMap& fileMap, Path analysisPath) :
    TTref_(Repository::getInstance()->getTypeTable()),
    fileMap_(fileMap),
    path_(analysisPath)
  {
  }
}