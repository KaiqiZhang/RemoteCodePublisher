/////////////////////////////////////////////////////////////////////
//  TypeAnal.cpp - analyze types in source code files              //
//  ver 1.1                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "TypeAnal.h"
#include <iostream>
#include <functional>
#include "../FileSystem/FileSystem.h"

using namespace CodeAnalysis;

//----< get the direct namespace of a type >-------------------------
/*
* only searching for non-member types as they are enough for dependency 
* analysis. Didn't add any types defined in class or struct because it 
* is not necessary for dependency analysis
*/

void TypeAnal::DFSforGlobal(ASTNode* pNode)
{
  static std::vector<std::string> vNamespace;
  if (pNode->type_ != "namespace")
    return;

  // push namespace
  vNamespace.push_back(pNode->name_);
  // add global declaration to type table
  for (auto iter = pNode->decl_.cbegin(); iter != pNode->decl_.cend(); iter++)
  {
    std::string typeName;
    // translate declaration name to string
    if (iter->declType_ == DeclType::typedefDecl)
      typeName = "typedef";
    else if (iter->declType_ == DeclType::aliasDecl)
      typeName = "alias";
    else if (iter->declType_ == DeclType::functionDecl)
      typeName = "function declaration";
    else if (iter->declType_ == DeclType::dataDecl)
      typeName = "data";
    else
      continue;

    TypeElement newType(typeName, iter->declName_, iter->package_, \
      FileSystem::Path::getRelativeFromPathToFile(path_, iter->path_));
    newType.setNamespaceVector(vNamespace);
    TTref_.add(newType);
  }
  // add class, struct, enum, function definition
  for (auto pChild : pNode->children_)
  {
    std::string type = pChild->type_;
    // do not add main(), although it's a global function
    if (type == "function" && pChild->name_ == "main")
      continue;
    // add type
    if (type == "class" || type == "struct" || type == "enum" || type == "function")
    {
      TypeElement newType(pChild->type_, pChild->name_, pChild->package_, \
        FileSystem::Path::getRelativeFromPathToFile(path_, pChild->path_));
      newType.setNamespaceVector(vNamespace);
      TTref_.add(newType);
    }
  }
  // DFS child namespace
  for (auto pChild : pNode->children_)
    DFSforGlobal(pChild);
  // pop namespace
  vNamespace.pop_back();
}

//----< do type analysis process >----------------------------------

void TypeAnal::doTypeAnal()
{
  std::cout << "\n    starting type analysis:\n";
  std::cout << "\n    scanning AST and identifying types:";
  std::cout << "\n    -----------------------------------------------";

  // get Abstract Syntax Tree root
  ASTNode* pRoot = ASTref_.root();

  // search for types
  DFSforGlobal(pRoot);

  // sort table by file names and paths
  TTref_.sortByFile();

  std::cout << "\n    scanning completed!\n";
  std::cout << "\n    type table size: " << TTref_.size() << "\n";
}

//----< Test Stub >--------------------------------------------------

#ifdef TEST_TYPEANAL

#include "../Parser/ConfigureParser.h"
#include "../FileSystem/FileSystem.h"
#include "../Display/Display.h"
#include <vector>
#include <string>
#define Util Utilities::StringHelper

void createAST(ConfigParseForCodeAnal& configure, Parser* pParser, std::vector<std::string>& files)
{
  Repository* pRepo = Repository::getInstance();

  for (int i = 0; i<files.size(); ++i)
  {
    std::string fileSpec = FileSystem::Path::getFullFileSpec(files[i]);
    std::string msg = "Processing file: " + fileSpec;
    Util::title(msg);
    Utilities::putline();

    pRepo->package() = FileSystem::Path::getName(fileSpec);
    pRepo->currentPath() = fileSpec;

    if (!configure.Attach(files[i]))
    {
      std::cout << "\n  could not open file " << fileSpec << std::endl;
      continue;
    }

    // now that parser is built, use it
    while (pParser->next())
      pParser->parse();
  }
  std::cout << "\n";
}

int main()
{
  Util::Title("Testing TypeAnal Class");
  Utilities::putline();

  // files to process
  std::vector<std::string> files;
  files.push_back("../TestFiles/Test1.h");
  files.push_back("../TestFiles/Test1.cpp");
  files.push_back("../TestFiles/Test2.cpp");

  // create paser instance
  ConfigParseForCodeAnal configure;
  Parser* pParser = configure.Build();
  if (!pParser)
  {
    std::cout << "\n\n  Parser not built\n\n";
    return 1;
  }

  // create Abstrat Syntax Tree from source files
  createAST(configure, pParser, files);

  // now we get AST
  TypeAnal typeAnal;
  typeAnal.doTypeAnal();

  // display type table
  Repository* pRepo = Repository::getInstance();
  TypeTable& tableRef = pRepo->getTypeTable();
  Display::showTypeTable(tableRef);
}

#endif