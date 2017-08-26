/////////////////////////////////////////////////////////////////////
//  TypeTable.cpp - a data structure that store types              //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to support code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "TypeTable.h"
#include <iomanip>
#include <algorithm>
#include <iostream>
#include "../FileSystem/FileSystem.h"

using namespace CodeAnalysis;

//----< find the types in type table with a specified name >---------
/*
* Considering types with same name but defined in different namespaces
* or overrided in different files, there may be several matched elements
* with the same name. So result is a vector.
*/

TypeTable::TypeElements TypeTable::findTypeName(const TypeName& name)
{
  TypeElements ret;

  for (auto iter = _records.begin(); iter != _records.end(); iter++)
  {
    if (name == iter->name())
      ret.push_back(*iter);
  }

  return ret;
}

//----< sort the type list by file names and file paths >------------

void TypeTable::sortByFile()
{
  // lambda to compare extension
  auto compExts = [](const TypeElement& first, const TypeElement& second) {
    return FileSystem::Path::getExt(first.path()) > FileSystem::Path::getExt(second.path());
  };
  // sort by extension names
  std::stable_sort(_records.begin(), _records.end(), compExts);

  // lambda to remove extension
  auto removeExt = [](std::string& name) {
    size_t extStartIndex = name.find_last_of('.');
    name = name.substr(0, extStartIndex);
  };
  // labmda to compare filenames(paths)
  auto compNames = [&removeExt](const TypeElement& first, const TypeElement& second) {
    std::string fnm = first.path();
    removeExt(fnm);
    std::string snm = second.path();
    removeExt(snm);
    return fnm < snm;
  };
  // sort by filenames
  std::stable_sort(_records.begin(), _records.end(), compNames);
}

//----< simply show types in type table >----------------------------

std::string TypeTable::show()
{
  std::ostringstream out;
  out << std::left;

  for (auto iter = _records.begin(); iter != _records.end(); iter++)
  {
    out << "\n  ";
    out << std::setw(25) << iter->name();
    out << std::setw(25) << iter->type();
    out << std::setw(20) << iter->package();
    out << std::setw(25) << iter->getNamespace();
    //out << std::setw(25) << iter->path();
  }
  out << "\n";

  return out.str();
}

//----< Test Stub >--------------------------------------------------

#ifdef TEST_TYPETABLE

int main()
{
  // create type table
  TypeTable table;

  // add type elements
  std::vector<std::string> nspace;
  nspace.push_back("Global Namespace");

  TypeElement type1("class", "Test1", "Test1.h", "C:\\Test1.h");
  type1.setNamespaceVector(nspace);

  TypeElement type2("alias", "Test1a", "Test1.cpp", "C:\\Test1.cpp");
  type2.setNamespaceVector(nspace);

  TypeElement type3("class", "Test2", "Test2.h", "C:\\Test2.h");
  nspace.push_back("Namespace1");
  type3.setNamespaceVector(nspace);

  TypeElement type4("class", "Test3", "Test2.h", "C:\\Test2.h");
  nspace.push_back("Namespace2");
  type4.setNamespaceVector(nspace);
  
  table.add(type4);
  table.add(type3);
  table.add(type2);
  table.add(type1);

  // display type table content
  std::cout << table.show();

  // sort by file name
  std::cout << "\n  Sort type table by file names and paths.\n";
  table.sortByFile();
  std::cout << table.show();

  // find a type with specified name
  TypeTable::TypeElements result = table.findTypeName("Test2");
  if (result.size() > 0)
    std::cout << "\n  Test2 found!";
  else
    std::cout << "\n  Test2 not found!";

  std::cout << "\n\n";
}

#endif