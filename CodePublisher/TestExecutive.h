#pragma once
/////////////////////////////////////////////////////////////////////
//  TestExecutive.h - demonstrate code publisher requirements      //
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
This package defines class TestCodePub, that demonstrates each of
the requirements in project #3 met.

Public Interface:
=================
TestCodePub test(fileMap, analPath, pubPath); // create an instance from parameters
test.DemoReq();                               // demonstrate all requirements

Build Process:
==============
Required files
- TypeAnal.h, TypeAnal.cpp
- DepAnal.h, DepAnal.cpp
- CodePublisher.h, CodePublisher.cpp
- Tokenizer.h, Tokenizer.cpp
- ActionsAndRules.h, ActionsAndRules.cpp
- Parser.h, Parser.cpp
- ScopeStack.h, ScopeStack.cpp
- ConfigureParser.cpp, ItokCollection.h
- SemiExpression.h, SemiExpression.cpp
- FileSystem.h, FileSystem.cpp
- Utilities.h, Utilities.cpp

Maintenance History:
====================
ver 1.0 : 07 Apr 2017
- first release

*/

#include <iostream>
#include "../Analyzer/TypeAnal.h"
#include "../Analyzer/DepAnal.h"
#include "CodePublisher.h"
#include "../Display/Display.h"
#include "../Utilities/Utilities.h"

using namespace CodeAnalysis;
using namespace CodePublisher;

class TestCodePub
{
public:
  using File = std::string;
  using Files = std::vector<File>;
  using Pattern = std::string;
  using Path = std::string;
  using FileMap = std::unordered_map<Pattern, Files>;
  using Utils = Utilities::StringHelper;

  TestCodePub(FileMap& fileMap, Path analysisPath, Path publishPath);

  void DemoReq();
  void DemoReq1();
  void DemoReq2();
  void DemoReq3();
  void DemoReq4();
  void DemoReq5();
  void DemoReq6();
  void DemoReq7();
  void DemoReq8();
  void DemoReq9();
  void DemoReq10();

  DepTable& depTable() { return depTable_; }

private:
  FileMap _fileMap;
  Path _analPath;
  Path _pubPath;
  DepTable depTable_;
  //NoSqlDb<std::string> _depDb;
};

//----< TestDepAnal constructor, initialize by parameters >------------

inline TestCodePub::TestCodePub(FileMap& fileMap, Path analysisPath, Path publishPath)
  : _fileMap(fileMap), _analPath(analysisPath), _pubPath(publishPath) {}


//----< Demonstrate requirement #1 >-----------------------------------

inline void TestCodePub::DemoReq1()
{
  Utils::sTitle("Demonstrating Req #1 - is C++", 3, 85);
  std::cout << "\n    Examine code in Visual Studio to verify that it is C++ Windows Console Projects.\n\n";
}

//----< Demonstrate requirement #2 >-----------------------------------

inline void TestCodePub::DemoReq2()
{
  Utils::sTitle("Demonstrating Req #2 - I/O library and memory", 3, 85);
  std::cout << "\n    Examine code in Visual Studio to verify that it uses C++ standard libraryis streams";
  std::cout << "\n    for all I/O and new and delete for all heap-based memory management.\n\n";
}

//----< Demonstrate requirement #3 >-----------------------------------

inline void TestCodePub::DemoReq3()
{
  Utils::sTitle("Demonstrating Req #3 - publisher program", 3, 85);

  // do type analysis
  TypeAnal typeAnal(_analPath);
  typeAnal.doTypeAnal();

  // display type table
  Repository* pRepo = Repository::getInstance();
  TypeTable& tableRef = pRepo->getTypeTable();
  Display::showTypeTable(tableRef);

  // do dependency analysis
  DepAnal depAnal(_fileMap, _analPath);
  depAnal.initDepTable();
  depAnal.doDepAnal();

  depTable_ = depAnal.depTable();
  // display dependency table
  Display::showDepTable(depAnal.depTable(), std::cout);

  // publish code
  Publisher publisher(depAnal.depTable(), _analPath, _pubPath);
  publisher.doPublish();

  std::cout << "\n";
}

//----< Demonstrate requirement #4 >-----------------------------------

inline void TestCodePub::DemoReq4()
{
  Utils::sTitle("Demonstrating Req #4 - expand and collapse", 3, 85);
  std::cout << "\n    Try the html pages to verify the expand and collapse features.\n\n";
}

//----< Demonstrate requirement #5 >-----------------------------------

inline void TestCodePub::DemoReq5()
{
  Utils::sTitle("Demonstrating Req #5 - CSS and Javascript files", 3, 85);
  std::cout << "\n    CSS file generated to dir: {publish dir}\\template.css.";
  std::cout << "\n    JavaScript file generated to dir: {publish dir}\\template.js.\n";

  std::cout << "\n";
}

inline void TestCodePub::DemoReq6()
{
  Utils::sTitle("Demonstrating Req #5 - links to CSS and Javascript", 3, 85);
  std::cout << "\n    Check the html files to verify the links to style sheet and\n";
  std::cout << "\n    JavaScipt file in the <head> section.\n\n";
}

inline void TestCodePub::DemoReq7()
{
  Utils::sTitle("Demonstrating Req #5 - HTML links to dependencies", 3, 85);
  std::cout << "\n    Check the html files in web browser to verify the links to dependencies.\n";
}

//----< Demonstrate requirement #8 >-----------------------------------

inline void TestCodePub::DemoReq8()
{
  Utils::sTitle("Demonstrating Req #8 - command line args", 3, 85);
  std::cout << "\n    Check the top of logs to verify that all required command line args are proceeded.\n";

  std::cout << "\n";
}

//----< Demonstrate requirement #9 >-----------------------------------

inline void TestCodePub::DemoReq9()
{
  Utils::sTitle("Demonstrating Req #9 - publish project #3", 3, 85);
  std::cout << "\n    Check the \"PubOutput\\\" in the CodePublisher dir to see the published files of Project #3.\n";

  std::cout << "\n";
}
//----< Demonstrate requirement #10 >----------------------------------

inline void TestCodePub::DemoReq10()
{
  Utils::sTitle("Demonstrating Req #10", 3, 85);
  std::cout << "\n    This demonstration shows that all requirements are satisfied.\n";

  std::cout << "\n";
}

//----< Demonstrate all requirements >---------------------------------

inline void TestCodePub::DemoReq()
{
  DemoReq1();
  DemoReq2();
  DemoReq3();
  DemoReq4();
  DemoReq5();
  DemoReq6();
  DemoReq7();
  DemoReq8();
  DemoReq9();
  DemoReq10();
}
