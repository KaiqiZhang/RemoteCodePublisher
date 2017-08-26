#pragma once
// Test1.h

#include <string>
#include <vector>
#include "../SemiExp/SemiExp.h"
#include "../Tokenizer/Tokenizer.h"

// test typedef
typedef int TypedefTest;

// test enum
enum EnumType
{
  EnumElement1, EnumElement2, EnumElement3
};

// test global function decleration in header file
void globalFunctionTestA();

namespace Test
{
  class Test1
  {
  public:
    Test1();
    Test1(const Test1&) = delete;
    Test1& operator=(const Test1&) = default;
    ~Test1();
    void doThing1() { /* don't do anything */ }
  private:
    Scanner::SemiExp se;
  };

  // test nested namespace
  namespace TestNested
  {
    inline void globalFunctionTestB()
    {
      // do nothing
    }
  }
}