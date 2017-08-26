/////////////////////////////////////////////////////////////////////
//  DepAnal.h - analyze dependency relationships between files     //
//  ver 1.1                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "DepAnal.h"
#include <string>
#include <functional>
#include "../Tokenizer/Tokenizer.h"
#include "../FileSystem/FileSystem.h"

using namespace CodeAnalysis;

//----< add all files to dependency table >-------------------------

void DepAnal::initDepTable()
{
  for (auto item : fileMap_)
  {
    for (auto file : item.second)
    {
      std::ifstream in(file);
      if (!in.good())
      {
        std::cout << "\n  can't open " << file << "\n\n";
        continue;
      }

      depTable_.addFile(FileSystem::Path::getRelativeFromPathToFile(path_, file));

      in.close();
    }
  }
}

//----< do analyze dependencies >-----------------------------------

void DepAnal::distTypes(std::unordered_set<std::string>& tokens, std::string file)
{
  file = FileSystem::Path::getRelativeFromPathToFile(path_, file);
  // check if tokens has type
  for (auto iter = TTref_.begin(); iter != TTref_.end(); iter++)
  {
    // check type name
    if (tokens.find(iter->name()) != tokens.end())  // found tok
    {
      // check namespace
      if (iter->getNamespace() == "Global Namespace" ||       // global namespace type
        tokens.find(iter->getNamespace()) != tokens.end())    // or corresponding namespace found
      {
        // find file element and add relationships
        depTable_.addDepFile(file, iter->path());
      }
    } // end if
  } // end for
}

//----< do analyze dependencies >-----------------------------------

void DepAnal::doDepAnal()
{
  std::cout << "\n    starting dependency analysis:\n";
  std::cout << "\n    scanning code files and checking with typetable:";
  std::cout << "\n    -----------------------------------------------";

  for (auto item : fileMap_)
  {
    for (auto file : item.second)
    {
      std::ifstream in(file);
      if (!in.good())
      {
        std::cout << "\n  can't open " << file << "\n\n";
        continue;
      }

      Scanner::Toker toker;
      toker.returnComments(false);
      toker.attach(&in);

      std::unordered_set<std::string> tokens;
      do
      {
        std::string tok = toker.getTok();
        // add tokens to unordered_set
        tokens.insert(tok);
      } while (in.good());

      // close file
      in.close();

      distTypes(tokens, file);
    } // end for
  } // end for

  std::cout << "\n    scanning completed!\n";
}

//----< Test Stub >--------------------------------------------------

#ifdef TEST_DEPANAL

#include "../Parser/ConfigureParser.h"
#include "../Display/Display.h"
#include "TypeAnal.h"

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

using File = std::string;
using Files = std::vector<File>;
using Pattern = std::string;
using Ext = std::string;
using FileMap = std::unordered_map<Pattern, Files>;

int main()
{
  // files to process
  std::vector<std::string> files;
  files.push_back("../TestFiles/Test1.h");
  files.push_back("../TestFiles/Test1.cpp");
  files.push_back("../TestFiles/Test2.cpp");

  // create file map
  FileMap fileMap;

  for (int i = 0; i<files.size(); ++i)
  {
    std::string fileSpec = FileSystem::Path::getFullFileSpec(files[i]);
    Ext ext = FileSystem::Path::getExt(fileSpec);
    Pattern p = "*." + ext;
    fileMap[p].push_back(fileSpec);
  }

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

  // create type table
  TypeAnal typeAnal;
  typeAnal.doTypeAnal();
  TypeTable& tableRef = Repository::getInstance()->getTypeTable();
  Display::showTypeTable(tableRef);
  std::cout << "\n";

  // do dependency analysis
  DepAnal depAnal(fileMap);
  depAnal.doDepAnal();

  // show dependencies
  Display::showDepTable(depAnal.depTable(), std::cout);
}

#endif
