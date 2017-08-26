// Test2.cpp

#include "../SemiExp/SemiExp.h"
#include "Test1.h"

int main()
{
  Scanner::SemiExp se(nullptr);
  se.show();

  // test dependency
  using namespace Test;
  Test1 t;
}