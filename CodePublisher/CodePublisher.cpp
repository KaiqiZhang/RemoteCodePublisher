/////////////////////////////////////////////////////////////////////
//  CodePublisher.cpp - publish code to html files                 //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform code publisher                  //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "CodePublisher.h"
#include <vector>
#include <ctime>
#include <stack>
#include "../FileSystem/FileSystem.h"

using namespace CodePublisher;

//----< Publisher constructor >--------------------------------------

Publisher::Publisher(DepTable& depTable, Path analysisPath, Path publishPath) :
  _depTable(depTable), _analPath(analysisPath), _pubPath(publishPath),
  ASTref_(Repository::getInstance()->AST())
{}

//----< DFS the AST to get all scope start and end line number >-----

void Publisher::DFS4Scope(ASTNode* pNode)
{
  //if (pNode->type_ != "namespace")
  //  return;

  for (auto pChild : pNode->children_)
  {
    std::string type = pChild->type_;

    if (type == "class" || type == "struct" || type == "function")
    {
      if (pChild->startLineCount_ == pChild->endLineCount_)
        continue;

      File file = FileSystem::Path::getRelativeFromPathToFile(_analPath, pChild->path_);

      _scopeTable.addStartLine(file, pChild->startLineCount_);
      _scopeTable.addEndLine(file, pChild->endLineCount_);
    }
  }

  // DFS child namespace
  for (auto pChild : pNode->children_)
    DFS4Scope(pChild);
}

//----< do publish codes >-------------------------------------------

void Publisher::doPublish()
{
  std::cout << "\n    starting publishing codes:\n";

  // get Abstract Syntax Tree root
  ASTNode* pRoot = ASTref_.root();
  DFS4Scope(pRoot);

  std::cout << "\n    generating css and javacript files.\n";
  genCssFile();
  genJsFile();

  std::cout << "\n    publishing codes to:" << _pubPath << "\n";
  genCodePages();
  genIndexPage();

  std::cout << "\n    publishing completed!\n";
}

//----< generate CSS file to the output dir >------------------------

void Publisher::genCssFile()
{
  if (!FileSystem::Directory::exists(_pubPath))
    FileSystem::Directory::create(_pubPath);

  File cssFile = FileSystem::Path::getAbsoluteFileSpec(".\\template.css", _pubPath);

  // open output file
  std::ofstream out(cssFile);
  if (!out.good())
  {
    std::cout << "\n  can't open " << cssFile << "\n\n";
    return;
  }

  // write content
  out << "body {\n";
  out << "  margin: 20px; color: black; background-color: #eee;\n";
  out << "  font-family: Consolas; font-weight: 600; font-size: 110%;\n";
  out << "}\n";
  out << "\n";
  out << ".indent {\n";
  out << "  margin-left: 20px; margin-right: 20px;\n";
  out << "}\n";
  out << "\n";
  out << "h4 {\n";
  out << "  margin-bottom: 3px; margin-top: 3px;\n";
  out << "}\n";
  out << "\n";
  out << "pre {\n";
  out << "  padding: 0; border: 0; margin: 0;\n";
  out << "}\n";

  // close file
  out.close();
}

//----< generate JavaScript file to the output dir >-----------------

void Publisher::genJsFile()
{
  if (!FileSystem::Directory::exists(_pubPath))
    FileSystem::Directory::create(_pubPath);

  File jsFile = FileSystem::Path::getAbsoluteFileSpec(".\\template.js", _pubPath);

  // open output file
  std::ofstream out(jsFile);
  if (!out.good())
  {
    std::cout << "\n  can't open " << jsFile << "\n\n";
    return;
  }

  // write content
  out << "function btn(id) {\n";
  out << "    if (id > 0) {\n";
  out << "        var unfold = document.getElementById(id);\n";
  out << "        var fold = document.getElementById(-parseInt(id));\n";
  out << "\n";
  out << "        unfold.style.display = 'none';\n";
  out << "        fold.style.display = '';\n";
  out << "    }\n";
  out << "\n";
  out << "    else {\n";
  out << "        var unfold = document.getElementById(-parseInt(id));\n";
  out << "        var fold = document.getElementById(id);\n";
  out << "\n";
  out << "        unfold.style.display = '';\n";
  out << "        fold.style.display = 'none';\n";
  out << "    }\n";
  out << "}\n";

  // close file
  out.close();
}

//----< generate html pages for each file >--------------------------

void Publisher::genCodePages()
{
  for (auto item : _depTable)
  {
    std::string file = item.first;
    std::string fullFileName = FileSystem::Path::getAbsoluteFileSpec(file, _analPath);

    // open input file
    std::ifstream in(fullFileName);
    if (!in.good())
    {
      std::cout << "\n  can't open " << fullFileName << "\n\n";
      continue;
    }

    // create path if not exist
    std::string webFileName = FileSystem::Path::getAbsoluteFileSpec(file, _pubPath) + ".htm";
    std::string webFilePath = FileSystem::Path::getPath(webFileName);
    if (!FileSystem::Directory::exists(webFilePath))
      FileSystem::Directory::create(webFilePath);

    // open output file
    std::ofstream out(webFileName);
    if (!out.good())
    {
      std::cout << "\n  can't open " << webFileName << "\n\n";
      continue;
    }

    genPrologue(webFileName, out);
    genHeader(file, out);
    genDepList(file, item.second, out);
    genCodeDiv(file, in, out);
    genFooter(out);

    out.close();
    in.close();
  }
}

//----< generate index html page to list all published files >-------

void Publisher::genIndexPage()
{
  File indexFile = ".\\index.htm";
  File indexFilePath = FileSystem::Path::getAbsoluteFileSpec(indexFile, _pubPath);

  // open output file
  std::ofstream out(indexFilePath);
  if (!out.good())
  {
    std::cout << "\n  can't open " << indexFilePath << "\n\n";
    return;
  }

  genPrologue(indexFile, out);  // prologue
  genHeader(indexFile, out);  // html header

                              // gen html list
  bool isFirst = true;
  for (auto item : _depTable)
  {
    std::string file = item.first;

    if (isFirst)
      isFirst = false;
    else
      out << "    <br>" << "\n";  // to next line

    File relPath = file + ".htm";
    out << "    <a href=\"" << relPath << "\">" << file << "</a>" << "\n";
  }

  genFooter(out);  // html footer

  out.close();  // close file

  std::cout << "\n    index web page: " << indexFilePath << "\n";
  /*std::string command("start \"\" \"" + indexFilePath + "\"");
  std::system(command.c_str());*/
}

//----< generate prologue for each html file >-----------------------

void Publisher::genPrologue(File file, std::ostream& out)
{
  out << "<!----------------------------------------------------------------------------" << "\n";
  out << "  " << FileSystem::Path::getName(file) << "\n";

  // get published time
  std::time_t curTime = std::time(nullptr);
  char timeStr[26];
  ctime_s(timeStr, sizeof timeStr, &curTime);
  out << "  Published " << timeStr;

  out << "  Kaiqi Zhang, CSE687 - Object Oriented Design, Spring 2017" << "\n";
  out << "----------------------------------------------------------------------------->" << "\n";
}

//----< generate html header, link to css and js file >--------------

void Publisher::genHeader(File file, std::ostream& out)
{
  std::string fileName = FileSystem::Path::getName(file);

  File cssFile = FileSystem::Path::getRelativeFromFileToFile(file, ".\\template.css");
  File JsFile = FileSystem::Path::getRelativeFromFileToFile(file, ".\\template.js");

  out << "<html>" << "\n";
  out << "<head>" << "\n";

  //out css link
  out << "  <link rel=\"stylesheet\" type=\"text/css\" href=\"" << cssFile << "\">" << "\n";

  //out js link
  out << "  <script src=\"" << JsFile << "\"></script>" << "\n";

  out << "</head>" << "\n";
  out << "<body>" << "\n";
  out << "  <h3>" << fileName << "</h3>" << "\n";
}

//----< generate html footer >---------------------------------------

void Publisher::genFooter(std::ostream& out)
{
  out << "</body>" << "\n";
  out << "</html>" << "\n";
}

//----< generate dependency links >----------------------------------

void Publisher::genDepList(File parent, DepTable::Deps deps, std::ostream& out)
{
  out << "  <hr />" << "\n";
  out << "  <div class=\"indent\">" << "\n";
  out << "    <h4>Dependencies:</h4>" << "\n";

  // out files
  bool isFirst = true;

  for (auto file : deps)
  {
    if (isFirst)
      isFirst = false;
    else
      out << "    <br>" << "\n";  // to next line

    std::string filename = FileSystem::Path::getName(file);
    File relPath = FileSystem::Path::getRelativeFromFileToFile(parent, file) + ".htm";
    out << "    <a href=\"" << relPath << "\">" << filename << "</a>" << "\n";
  }

  out << "  </div>" << "\n";
  out << "  <hr />" << "\n";
}

//----< add line no. and javascipt prefix to every line of code >-----

void Publisher::addCodeLinePrefix(File file, std::vector<std::string>& lines)
{
  int btnId = 1;
  std::stack<int> scopeFirstLineNumber;
  std::stack<std::string> scopeFirstLine;
  std::stack<int> scopeBtnId;

  for (size_t i = 0; i < lines.size(); i++)
  {
    std::ostringstream temp;
    temp << std::right;

    ScopeTable::ScopeType type = _scopeTable.lineType(file, i + 1);

    if (type == ScopeTable::start)  // scope begin
    {
      scopeFirstLine.push(lines[i]);
      scopeFirstLineNumber.push(i + 1);
      scopeBtnId.push(btnId);

      temp << "</pre>" << "\n";
      temp << "<pre id=\"" << btnId++ << "\" onclick=\"btn(this.id)\">" << "\n";
      temp << std::setw(3) << i + 1 << " [-]  " << lines[i] << "\n";
    }
    else if (type == ScopeTable::end)  // scope end
    {
      temp << std::setw(3) << i + 1 << "  -   " << lines[i] << "\n";

      // insert collapsed lines
      temp << "</pre>" << "\n";
      temp << "<pre id=\"" << -scopeBtnId.top() << "\" style=\"display:none\" onclick=\"btn(this.id)\">" << "\n";
      temp << std::setw(3) << scopeFirstLineNumber.top() << " [+]  " << scopeFirstLine.top().c_str() << " ..." << "\n";
      temp << "</pre>" << "\n";
      temp << "<pre>" << "\n";

      scopeBtnId.pop();
      scopeFirstLineNumber.pop();
      scopeFirstLine.pop();
    }
    else if (!scopeBtnId.empty())
      temp << std::setw(3) << i + 1 << "  |   " << lines[i] << "\n";
    else
      temp << std::setw(3) << i + 1 << "      " << lines[i] << "\n";

    lines[i] = temp.str();
  }
}

//----< generate code div in html >----------------------------------

void Publisher::genCodeDiv(File file, std::istream& in, std::ostream& out)
{
  // code div start
  out << "<pre>" << "\n";

  // step 1: get original lines & replace < >
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(in, line))
  {
    StringReplace(line, "<", "&lt");
    StringReplace(line, ">", "&gt");
    lines.push_back(line);
  }

  // step 2: add prefix
  addCodeLinePrefix(file, lines);

  // step 3: dump all lines
  for (std::string nline : lines)
    out << nline.c_str();

  // code div end
  out << "</pre>" << "\n";
}

//----< replace a specified characters in a string >-----------------

void Publisher::StringReplace(std::string &strBase, std::string strSrc, std::string strDes)
{
  std::string::size_type srcLen = strSrc.size();
  std::string::size_type desLen = strDes.size();
  std::string::size_type pos = strBase.find(strSrc, 0);

  while ((pos != std::string::npos))
  {
    strBase.replace(pos, srcLen, strDes);
    pos = strBase.find(strSrc, (pos + desLen));
  }
}

//----< conduct code publishing >------------------------------------
#ifdef TEST_CODEPUBLISHER

#include <fstream>
#include "../Analyzer/Executive.h"
#include "TestExecutive.h"
#include "../Logger/Logger.h"

int main(int argc, char* argv[])
{
  using namespace CodeAnalysis;
  using Rslt = Logging::StaticLogger<0>;  // use for application results
  CodeAnalysisExecutive exec;

  try {
    bool succeeded = exec.ProcessCommandLine(argc, argv);
    if (!succeeded) return 1;
    exec.setDisplayModes();
    exec.startLogger(std::cout);

    std::ostringstream tOut("CodePublisher - Version 1.0");
    Utils::sTitle(tOut.str(), 3, 92, tOut, '=');
    Rslt::write(tOut.str());

    Rslt::write("\n     " + exec.systemTime());
    Rslt::flush();
    exec.showCommandLineArguments(argc, argv);
    Rslt::write("\n");

    exec.getSourceFiles();
    exec.processSourceCode(true);
    exec.complexityAnalysis();
    exec.dispatchOptionalDisplays();
    exec.flushLogger();
    Rslt::write("\n");
    std::ostringstream out;
    out << "\n  " << std::setw(10) << "searched" << std::setw(6) << exec.numDirs() << " dirs";
    out << "\n  " << std::setw(10) << "processed" << std::setw(6) << exec.numFiles() << " files";
    Rslt::write(out.str());
    Rslt::write("\n");
    exec.stopLogger();
    std::cout << "\n    Code Analysis completed";
    std::cout << "\n";

    TestCodePub test(exec.getFileMap(), exec.getAnalysisPath(), exec.getPublishDir());
    test.DemoReq();
    std::cout << "\n\n";
  }
  catch (std::exception& except)
  {
    exec.flushLogger();
    std::cout << "\n\n  caught exception in Executive::main: " + std::string(except.what()) + "\n\n";
    exec.stopLogger();
    return 1;
  }
  return 0;
}

#endif