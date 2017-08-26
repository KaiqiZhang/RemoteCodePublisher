#pragma once
/////////////////////////////////////////////////////////////////////
//  TypeTable.h - a data structure that store types                //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to support code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a TypeTable class and an TypeElement class.
Those provides support for storing type informations during source
code dependency analysis.

Public Interface:
=================
TypeTable table;                    // create a type talbe instance
TypeElement elem;                   // create a type element
table.add(elem);                    // add a type to table
table.findTypeName(name);           // find a list of type by name
table.sortByFile();                 // sort table by file name/path
table.show();                       // a simplified show of table content

Build Process:
==============
Required files
- TypeTable.h, TypeTable.cpp
- FileSystem.h, FileSystem.cpp

Maintenance History:
====================
ver 1.0 : 13 Mar 2017
- first release

*/

#include <string>
#include <sstream>
#include <vector>

namespace CodeAnalysis
{
  ///////////////////////////////////////////////////////////////////
  // TypeElement class represents a type record in type table

  class TypeElement
  {
  public:
    using Type = std::string;
    using TypeName = std::string;
    using Namespace = std::string;
    using Package = std::string;
    using Path = std::string;

    TypeElement(Type type, TypeName name, Package package, Path path) :
      _type(type), _name(name), _package(package), _path(path) {};

    Type type() const { return _type; }
    TypeName name() const { return _name; }
    Package package() const { return _package; }
    Path path() const { return _path; }

    void setNamespaceVector(std::vector<Namespace>& nspacevec) { _nspace = nspacevec; }
    Namespace getNamespace();
    Namespace getFullNamespace();

  private:
    Type _type;
    TypeName _name;
    Package _package;
    Path _path;
    std::vector<Namespace> _nspace;
  };

  //----< get the direct namespace of a type >-------------------------

  inline TypeElement::Namespace TypeElement::getNamespace()
  {
    if (_nspace.size() > 0)
      return _nspace.back();
    else
      return "";
  }

  //----< get the full namespace of a type >---------------------------

  inline TypeElement::Namespace TypeElement::getFullNamespace()
  {
    std::ostringstream out;

    if (_nspace.size() > 0)
    {
      for (auto iter = _nspace.cbegin(); iter != _nspace.cend(); iter++)
      {
        if (iter != _nspace.cbegin()) out << "::";
        out << (*iter);
      }
    }
    
    return out.str();
  }

  ///////////////////////////////////////////////////////////////////
  // TypeTalbe class stores different types represented by TypeElement

  class TypeTable
  {
  public:
    using TypeName = std::string;
    using TypeElements = std::vector<TypeElement>;

    using iterator = typename TypeElements::iterator;

    void add(const TypeElement& pElement) { _records.push_back(pElement); }
    TypeElement& operator[](size_t i);
    TypeElement operator[](size_t i) const;
    TypeElements findTypeName(const TypeName& name);    // same typenames in different namespaces
    TypeElements getRecords() { return _records; }
    void sortByFile();
    std::string show();   // simple show function. just for test use

    const size_t size() { return _records.size(); }
    iterator begin() { return _records.begin(); }
    iterator end() { return _records.end(); }

  private:
    TypeElements _records;
  };

  //----< index non-const type elements >------------------------------

  inline TypeElement& TypeTable::operator[](size_t i)
  {
    if (i < 0 || i >= _records.size())
      throw(std::exception("index out of range"));
    return _records[i];
  }

  //----< index const type elements >----------------------------------

  inline TypeElement TypeTable::operator[](size_t i) const
  {
    if (i < 0 || i >= _records.size())
      throw(std::exception("index out of range"));
    return _records[i];
  }

}
