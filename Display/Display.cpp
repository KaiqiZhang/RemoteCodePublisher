/////////////////////////////////////////////////////////////////////
//  Display.cpp -  display type table, dependency relations, and   //
//                 strongly connected components                   //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code dependency analysis        //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include <iomanip>
#include "Display.h"
#include "../Utilities/Utilities.h"
#include "../FileSystem/FileSystem.h"

using namespace CodeAnalysis;

//----< show type table contents >----------------------------------

void Display::showTypeTable(TypeTable& table, std::ostream& out)
{
  static std::string path;
  // show title
  std::string title = "Type Table - Display all non-member type definitions in the directory";
  Utilities::StringHelper::sTitle(title, 3, 90, out, '=');

  for (auto iter = table.begin(); iter != table.end(); iter++)
  {
    // show header
    if (iter->path() != path)
    {
      path = iter->path();
      out << "\n ";
      showTypeTableHeader();  // show every file
    }

    // show elements
    showTypeTableElement(*iter);
  }

  out << "\n";
}

//----< show type table header >------------------------------------

void Display::showTypeTableHeader(std::ostream& out)
{
  out << std::right;
  out << "\n ";
  out << std::setw(20) << "file name";
  out << std::setw(24) << "type";
  out << std::setw(30) << "name";
  out << std::setw(20) << "namespace";
  out << std::right;
  out << "\n  ";
  out << std::setw(20) << "------------------";
  out << std::setw(24) << "----------------------";
  out << std::setw(30) << "----------------------------";
  out << std::setw(20) << "------------------";
}

//----< show type table element >------------------------------------

void Display::showTypeTableElement(TypeElement& elem, std::ostream& out)
{
  out << std::right;
  out << "\n ";
  out << std::setw(20) << trunc(elem.package(), 18);
  out << std::setw(24) << trunc(elem.type(), 22);
  out << std::setw(30) << trunc(elem.name(), 28);
  out << std::setw(20) << trunc(elem.getNamespace(), 18);
  out << std::right;
}

//----< show file dependencies >-------------------------------------

void Display::showDepTable(DepTable& depTable, std::ostream& out)
{
  // show title
  std::string title = "File Dependencies";
  Utilities::StringHelper::sTitle(title, 3, 90, out, '=');

  using Item = DepTable::Item;

  for (Item item : depTable)
  {
    showDepHeader(out);
    showDepFile(item.first, 0, out);
    
    out << std::left;
    out << "\n     ";
    out << std::setw(20) << "dependencies:";
    out << "size: " << item.second.size();

    // display children file
    for (auto child : item.second)
    {
      showDepFile(child, 2, out);
    }

    out << "\n";
  }

  std::cout << "\n    Depend table size: " << depTable.size();
  std::cout << "\n";
}

//----< show file dependencies header >-------------------------------

void Display::showDepHeader(std::ostream& out)
{
  out << std::left;
  out << "\n     ";
  out << std::setw(20) << "file name";
  out << std::setw(72) << "path";
  out << std::left;
  out << "\n    ";
  out << std::setw(20) << std::string(18, '-').c_str();
  out << std::setw(72) << std::string(72, '-').c_str();
}

//----< show file dependencies element >------------------------------

void Display::showDepFile(DepTable::File file, size_t indent, std::ostream& out)
{
  out << std::left;
  out << "\n     ";
  out << std::string(indent, ' ').c_str();
  out << std::setw(20) << trunc(FileSystem::Path::getName(file), 18);
  out << std::setw(72) << trunc(file, 70);
}

//----< Test Stub >--------------------------------------------------

#ifdef TEST_DISPLAY

int main()
{
  // create nosql database
  std::string file1 = "C:\\Users\\Kaiqi Zhang\\OneDrive\\Documents\\CSE687-OOD\\Project2HelpS17\\CodeAnalyzerEx\\Utilities\\Utilities.h";
  std::string file2 = "C:\\Users\\Kaiqi Zhang\\OneDrive\\Documents\\CSE687-OOD\\Project2HelpS17\\CodeAnalyzerEx\\Utilities\\Utilities.cpp";
  std::string file3 = "C:\\Users\\Kaiqi Zhang\\OneDrive\\Documents\\CSE687-OOD\\Project2HelpS17\\CodeAnalyzerEx\\TestFiles\\Test1.h";
  std::string file4 = "C:\\Users\\Kaiqi Zhang\\OneDrive\\Documents\\CSE687-OOD\\Project2HelpS17\\CodeAnalyzerEx\\TestFiles\\Test1.cpp";

  DepTable depTable;

  depTable.addDepFile(file1, file2);
  depTable.addDepFile(file1, file3);
  depTable.addDepFile(file1, file4);
  depTable.addDepFile(file2, file3);
  depTable.addDepFile(file2, file4);
  depTable.addDepFile(file3, file4);

  // display
  Display::showDepTable(depTable, std::cout);
}

#endif