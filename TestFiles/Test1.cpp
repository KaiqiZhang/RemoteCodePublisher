// Test1.cpp

#include "Test1.h"
#include <iostream>
#include <sstream>

// test global data in cpp file
int globalDataTest;

// test template global data
std::vector<std::string> globalDataX;

// test global function implemention in cpp file with decleration in header file
void globalFunctionTestA()
{
  // do nothing
}

// test member function implementation in cpp file
Test::Test1::Test1() {}

Test::Test1::~Test1() {}

// test global function implemention in cpp file
template<typename T>
std::string toString(const T& t)
{
  std::ostringstream out;
  out << t;
  return out.str();
}

// test namespace using
using namespace Scanner;
// test alias
using Tst = Test::Test1;

#ifdef TEST_TEST1

int main()
{
  SemiExp se1;
  std::cout << "\n  " << toString<size_t>(se1.length());
  std::cout << "\n\n";
}
#endif
