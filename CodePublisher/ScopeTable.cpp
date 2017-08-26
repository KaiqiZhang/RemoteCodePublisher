/////////////////////////////////////////////////////////////////////
//  ScopeTable.cpp - a data structure that store scope start & end //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to support code publisher                  //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "ScopeTable.h"
#include <iostream>

using namespace CodePublisher;

//----< add 'lineNo' as scope start line in 'file'  >----------------

void ScopeTable::addStartLine(File file, size_t lineNo)
{
  _store[file][lineNo] = ScopeType::start;
}

//----< add 'lineNo' as scope end line in 'file'  >------------------

void ScopeTable::addEndLine(File file, size_t lineNo)
{
  _store[file][lineNo] = ScopeType::end;
}

//----< get the scope type of 'lineNo' in 'file' >-------------------

ScopeTable::ScopeType ScopeTable::lineType(File file, size_t lineNo)
{
  if (_store.find(file) == _store.end())
  {
    return ScopeType::none;  // file does not exist
  }
  else
  {
    ScopeTableOne& scopeTableOne = _store[file];

    if (scopeTableOne.find(lineNo) == scopeTableOne.end())
    {
      return ScopeType::none;  // not scope start, not scope end
    }
    else
    {
      return scopeTableOne[lineNo];
    }
  }
}

//----< show content in scope table >--------------------------------

void ScopeTable::show()
{
  for (auto scopeTableOne : _store)
  {
    std::cout << "\n  File: " << scopeTableOne.first.c_str();

    for (auto item : scopeTableOne.second)
    {
      std::cout << "\n  line: " << item.first << " type: " << item.second;
    }
  }

  std::cout << "\n";
}

//----< Test Stub >--------------------------------------------------

#ifdef TEST_SCOPETABLE

#include "../Utilities/Utilities.h"

int main()
{
  Utilities::StringHelper::Title("Testing ScopeTable Class");
  Utilities::putline();

  ScopeTable table;
  table.addStartLine(".\\file.h", 1);
  table.addEndLine(".\\file.h", 3);

  table.show();

  // test lineType function
  std::cout << "\n  line:" << 1 << " type:" << table.lineType(".\\file.h", 1);
  std::cout << "\n  line:" << 2 << " type:" << table.lineType(".\\file.h", 2);
  std::cout << "\n  line:" << 3 << " type:" << table.lineType(".\\file.h", 3);
  std::cout << "\n  line:" << 4 << " type:" << table.lineType(".\\file.h", 4);
}

#endif