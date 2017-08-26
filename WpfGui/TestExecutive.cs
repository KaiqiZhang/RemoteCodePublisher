/////////////////////////////////////////////////////////////////////
//  TestExecutive.cs - demonstrate code publisher requirements     //
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
the requirements in project #4 met.

Public Interface:
=================
TestExecutive exec = new TestExecutive();  // create an instance
exec.DemoReq(window);                      // demonstrate all requirement

Build Process:
==============
Required files
- TestExecutive.cs
- MainWindow.xaml.cs

Maintenance History:
====================
ver 1.0 : 06 May 2017
- first release

*/
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace WpfGui
{
    class TestExecutive
    {
        //----< Demonstrate all requirements >---------------------------------
        public void DemoReq(MainWindow wnd)
        {
            sTitle("RemoteCodePublisher - Version 1.0", 3, 92, '=');
            Console.Write("\n    Automated Test Mode");
            Console.Write("\n    Path: {0}", Directory.GetCurrentDirectory());
            Console.Write("\n\n");

            DemoReq1();
            DemoReq2();
            DemoReq3(wnd);
            DemoReq6();
            DemoReq7();
            DemoReq8();
            DemoReq9();
            DemoReq10(wnd);
        }

        //----< write minor title to console >---------------------------------
        private void sTitle(string src, int offset, int width, char underline = '-')
        {
            if (width < src.Length)
                width = src.Length;
            int lwidth = (width - src.Length) / 2;

            string line1 = "\n  " + new String(' ', offset) + new String(' ', lwidth) + src;
            string line2 = "\n " + new String(' ', offset) + new String(underline, width + 2);

            Console.Write(line1);
            Console.Write(line2);
        }

        //----< Demonstrate requirement #1 >-----------------------------------
        private void DemoReq1()
        {
            sTitle("Demonstrating Req #1 - is C++ & WPF", 3, 85);
            Console.Write("\n    Examine code in Visual Studio to verify that it is C++ Windows Console Projects and WPF");
            Console.Write("\n    is usedand WPF is used to provide a required GUI.\n\n");
        }

        //----< Demonstrate requirement #2 >-----------------------------------
        private void DemoReq2()
        {
            sTitle("Demonstrating Req #2 - I/O library and memory", 3, 85);
            Console.Write("\n    Examine code in Visual Studio to verify that it uses C++ standard libraryis streams");
            Console.Write("\n    for all I/O and new and delete for all heap-based memory management.\n\n");
        }

        //----< Demonstrate requirement #3 #4 #5 >-----------------------------
        private void DemoReq3(MainWindow wnd)
        {
            sTitle("Demonstrating Req #3 #4 - Publish program", 3, 85);
            sTitle("Demonstrating Req #5 - Client program", 3, 85);

            DemoReq3_1(wnd);
            DemoReq3_2(wnd);
            DemoReq3_3();
        }

        //----< Demonstrate requirement #3 >-----------------------------------
        private void DemoReq3_1(MainWindow wnd)
        {
            Console.Write("\n    1st: upload all the files and directories in the test path...");

            string[] files = Directory.GetFiles(".", "*.*").Where(file => file.ToLower().EndsWith(".h") || file.ToLower().EndsWith(".cpp")).ToArray();
            string[] dirs = Directory.GetDirectories(".");

            List<string> fileList = new List<string>();
            foreach (var file in files)
                fileList.Add(file.Substring(2));

            foreach (var dir in dirs)
                foreach (string file in wnd.recursiveGetFiles(dir))
                    fileList.Add(file.Substring(2));

            wnd.sendFiles(ref fileList, "");

            Console.Write("\n    Upload successful.\n");
        }

        //----< Demonstrate requirement #3 >-----------------------------------
        private void DemoReq3_2(MainWindow wnd)
        {
            Console.Write("\n    2nd: publish the source files that were uploaded...");

            wnd.btnPublish_Click(null, null);

            Console.Write("\n    Publish successful.\n");
        }

        //----< Demonstrate requirement #3 >-----------------------------------
        private void DemoReq3_3()
        {
            Console.Write("\n    3rd: list all the htm files that were published...");

            // automatic refresh remoteListView after published

            Console.Write("\n    List successful. Please check the right side of the GUI.\n");
        }

        //----< Demonstrate requirement #6 >-----------------------------------
        private void DemoReq6()
        {
            sTitle("Demonstrating Req #6 - Socket based communication", 3, 85);
            Console.Write("\n    Examine code in Visual Studio to verify that socket is used in communication between");
            Console.Write("\n    processes or machines.\n\n");
        }

        //----< Demonstrate requirement #7 >-----------------------------------
        private void DemoReq7()
        {
            sTitle("Demonstrating Req #7 - HTTP style message", 3, 85);
            Console.Write("\n    Examine code in Visual Studio to verify that HTTP style message is used in communication\n\n");
        }

        //----< Demonstrate requirement #8 >-----------------------------------
        private void DemoReq8()
        {
            sTitle("Demonstrating Req #8 - Sending and receiving streams of bytes", 3, 85);
            Console.Write("\n    Streaming of bytes is used to send and receive files.");
            Console.Write("\n    Examine code in Visual Studio to verify that Streaming of bytes is used in communication\n\n");
        }

        //----< Demonstrate requirement #9 >-----------------------------------
        private void DemoReq9()
        {
            sTitle("Demonstrating Req #9 - Automated test suite", 3, 85);
            Console.Write("\n    This demonstration shows that all requirements are satisfied.\n\n");
        }

        //----< Demonstrate requirement #10 >----------------------------------
        private void DemoReq10(MainWindow wnd)
        {
            sTitle("Demonstrating Req #10 - Lazy download", 3, 85);
            Console.Write("\n    If one file is requested, all the files directly or indrectly linked to it will be download.");
            Console.Write("\n    For example, download and open Grandparent.h, the linked file Child.h will be downloaded too.\n");

            Console.Write("\n    Download and open Grandparent.h ...\n");
            String msg = "OpenFile," + ".\\Grandparent.h";
            if (wnd.shim != null)
                wnd.shim.PostMessage(msg);

            Console.Write("\n    If index file is requested, then all the files will be downloaded.");
            Console.Write("\n");
        }
    }
}
