/////////////////////////////////////////////////////////////////////
//  MainWindow.xaml.cs - GUI for remote code publisher             //
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
This package defines class MainWindow, that present a WPF GUI for remote
code publisher client.

Public Interface:
=================
MainWindow wnd = new MainWindow();  // create an gui instance
wnd.doTest = true;                  // set the client to test mode
wnd.testPath = e.Args[0];           // set test file directory
wnd.Show();                         // show gui window

Build Process:
==============
Required files
- MainWindow.xaml.cs
- TestExecutive.cs
- CLIShim.h, CLIShim.cpp

Maintenance History:
====================
ver 1.0 : 06 May 2017
- first release

*/
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace WpfGui
{
    /// <summary>
    /// MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public Shim shim { get; set; }           // Shim interface to C++ codes
        public bool doTest { get; set; }         // if startup with test mode
        public string testPath { get; set; }     // test codes path for test mode

        Thread tRcv = null;
        Thread tTest = null;
        String currentRemoteDir = ".";

        //----< MainWindow constructor >-----------------------------------------
        public MainWindow()
        {
            InitializeComponent();

            // create new Shim instance
            shim = new Shim();
            doTest = false;
            testPath = "";

            // refresh local files list view after loaded
            refreshLocalListView();
        }

        //----< data source for local list view >---------------------------------
        public void refreshLocalListView()
        {
            string path = Directory.GetCurrentDirectory();

            // also refresh text box
            tbxLocalFolder.Text = path;

            localListView.Items.Clear();
            string[] files = Directory.GetFiles(path, "*.*").Where(file => file.ToLower().EndsWith(".h") || file.ToLower().EndsWith(".cpp")).ToArray();
            string[] dirs = Directory.GetDirectories(path);

            // add a link to parent directory
            if (path != Directory.GetDirectoryRoot(path))
            {
                FileDirItem backItem = new FileDirItem() { Name = "..", Icon = "pack://siteoforigin:,,,/Resources/folderclosed.png", isFolder = true };
                localListView.Items.Add(backItem);
            }

            // add dirs to list view
            for (int i = 0; i < dirs.Length; i++)
            {
                FileInfo dir = new FileInfo(dirs[i]);
                FileDirItem item = new FileDirItem() { Name = dir.Name, LastWriteTime = dir.LastWriteTime.ToString(), isFolder = true };
                item.Icon = "pack://siteoforigin:,,,/Resources/folderclosed.png";
                localListView.Items.Add(item);
            }

            // add files to list view
            for (int i = 0; i < files.Length; i++)
            {
                FileInfo file = new FileInfo(files[i]);
                FileDirItem item = new FileDirItem() { Name = file.Name, LastWriteTime = file.LastWriteTime.ToString(), isFolder = false };
                item.Icon = "pack://siteoforigin:,,,/Resources/file.png";
                localListView.Items.Add(item);
            }
        }

        //----< data source for remote list view >--------------------------------
        public void refreshRemoteListView(string[] fileDirs)
        {
            remoteListView.Items.Clear();

            if (currentRemoteDir != ".")
            {
                FileDirItem backItem = new FileDirItem() { Name = "..", Icon = "pack://siteoforigin:,,,/Resources/folderclosed.png", isFolder = true };
                remoteListView.Items.Add(backItem);
            }

            // add dirs
            foreach (string fileDir in fileDirs)
            {
                if (fileDir != "")
                {
                    string[] fileInfo = fileDir.Split(new char[] { ';' });

                    if (fileInfo[2] == "d")
                    {
                        FileDirItem item = new FileDirItem() { Name = fileInfo[0], LastWriteTime = fileInfo[1], isFolder = true };
                        item.Icon = "pack://siteoforigin:,,,/Resources/folderclosed.png";
                        remoteListView.Items.Add(item);
                    }
                }
            }

            // add files
            foreach (string fileDir in fileDirs)
            {
                if (fileDir != "")
                {
                    string[] fileInfo = fileDir.Split(new char[] { ';' });

                    if (fileInfo[2] == "f")
                    {
                        FileDirItem item = new FileDirItem() { Name = fileInfo[0], LastWriteTime = fileInfo[1], isFolder = false };
                        item.Icon = "pack://siteoforigin:,,,/Resources/file.png";
                        remoteListView.Items.Add(item);
                    }
                }
            }
        }

        //----< open button click handler >-------------------------------------
        private void btnOpenFolder_Click(object sender, RoutedEventArgs e)
        {
            System.Windows.Forms.FolderBrowserDialog folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            folderBrowserDialog.ShowNewFolderButton = false;
            folderBrowserDialog.SelectedPath = Directory.GetCurrentDirectory();

            if (folderBrowserDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                Directory.SetCurrentDirectory(folderBrowserDialog.SelectedPath);
                refreshLocalListView();
            }
        }

        //----< file item double click handler >-------------------------------------
        private void localListViewItem_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            ListViewItem item = sender as ListViewItem;
            object obj = item.Content;
            if (obj.GetType() == typeof(FileDirItem))
            {
                FileDirItem fileDirItem = (FileDirItem)obj;
                if (fileDirItem.isFolder)
                {
                    if (fileDirItem.Name == "..")
                    {
                        DirectoryInfo dirInfo = new DirectoryInfo(Directory.GetCurrentDirectory());
                        string nextPath = dirInfo.Parent.FullName;
                        Directory.SetCurrentDirectory(nextPath);
                    } else
                    {
                        string nextPath = Directory.GetCurrentDirectory() + "/" + fileDirItem.Name;
                        Directory.SetCurrentDirectory(nextPath);
                    }

                    refreshLocalListView();
                }
            }
        }

        //----< file item double click handler >-------------------------------------
        private void remoteListViewItem_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            ListViewItem item = sender as ListViewItem;
            object obj = item.Content;
            if (obj.GetType() == typeof(FileDirItem))
            {
                FileDirItem fileDirItem = (FileDirItem)obj;
                if (fileDirItem.isFolder)
                {
                    if (fileDirItem.Name == ".." && currentRemoteDir != ".")
                    {
                        currentRemoteDir = currentRemoteDir.Substring(0, currentRemoteDir.LastIndexOf('\\'));
                    } else
                    {
                        currentRemoteDir += ("\\" + fileDirItem.Name);
                    }
                    
                    btnRefreshRemote_Click(null, null);
                }
            }
        }

        //----< upload button click handler >-------------------------------------
        private void btnUpload_Click(object sender, RoutedEventArgs e)
        {
            List<string> fileList = new List<string>();

            if (localListView.SelectedItems.Count > 0)
            {
                foreach (var item in localListView.SelectedItems)
                {
                    if (item.GetType() == typeof(FileDirItem))
                    {
                        FileDirItem fileDirItem = (FileDirItem)item;

                        if (fileDirItem.isFolder && fileDirItem.Name != "..")
                        {
                            foreach (string file in recursiveGetFiles(fileDirItem.Name))
                                fileList.Add(file);
                        }
                        else if (fileDirItem.Name != "..")
                        {
                            fileList.Add(fileDirItem.Name);
                        }
                    }
                }
            }

            if (fileList.Count == 0)
                MessageBox.Show("No file selected!", "Upload", MessageBoxButton.OK, MessageBoxImage.Information);
            else
            {
                string msgBoxString = String.Format("Upload {0} files?", fileList.Count);
                InputDialog inputDialog = new InputDialog(msgBoxString + "\nInput the name to create a new dir? Or leave it blank.");
                if (inputDialog.ShowDialog() == true)
                {
                    sendFiles(ref fileList, inputDialog.Answer);

                    // do publish
                    if (cbxPublish.IsChecked == true)
                    {
                        btnPublish_Click(null, null);
                    } else
                    {
                        // if don't publish, refresh list view
                        Thread.Sleep(100);
                        btnRefreshRemote_Click(null, null);
                    }
                }
            }
        }

        //----< send all the file in fileList to server, optional to create new directory >----
        public void sendFiles(ref List<string> fileList, string newDir)
        {
            foreach (string file in fileList)
            {
                String msg = "Upload," + (Directory.GetCurrentDirectory() + "\\" + file + ",");
                
                if (newDir == "")
                    msg += file;
                else
                    msg += (newDir + "\\" + file);

                shim.PostMessage(msg);
            }
        }

        //----< button click handler >------------------------------------------------
        public void btnRefreshRemote_Click(object sender, RoutedEventArgs e)
        {
            tbxRemoteFolder.Text = currentRemoteDir;
            String msg = "GetFileDirs," + currentRemoteDir + ",";

            if (cbxNoParent.IsChecked == true)
                msg += "NoParent";
            else
                msg += "All";

            shim.PostMessage(msg);
        }

        //----< button click handler >------------------------------------------------
        private void btnOpenRemote_Click(object sender, RoutedEventArgs e)
        {
            if (remoteListView.SelectedItems.Count == 0)
            {
                MessageBox.Show("No file selected!", "Open", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            var item = remoteListView.SelectedItem;
            if (item.GetType() == typeof(FileDirItem))
            {
                FileDirItem fileDirItem = (FileDirItem)item;

                if (fileDirItem.isFolder)
                {
                    MessageBox.Show("No file selected!", "Open", MessageBoxButton.OK, MessageBoxImage.Information);
                } else
                {
                    String filePath = currentRemoteDir + "\\" + fileDirItem.Name;
                    String msg = "OpenFile," + filePath;
                    shim.PostMessage(msg);
                }
            } 
        }

        //----< button click handler >------------------------------------------------
        private void btnDelRemote_Click(object sender, RoutedEventArgs e)
        {
            if (remoteListView.SelectedItems.Count == 0)
            {
                MessageBox.Show("No file selected!", "Open", MessageBoxButton.OK, MessageBoxImage.Information);
                return;
            }

            var item = remoteListView.SelectedItem;
            if (item.GetType() == typeof(FileDirItem))
            {
                FileDirItem fileDirItem = (FileDirItem)item;

                if (fileDirItem.Name == "..")
                {
                    MessageBox.Show("No file selected!", "Open", MessageBoxButton.OK, MessageBoxImage.Information);
                    return;
                }

                String filePath = currentRemoteDir + "\\" + fileDirItem.Name;
                String msg;
                if (fileDirItem.isFolder)
                    msg = "DelDir," + filePath;
                else
                    msg = "DelFile," + filePath;

                shim.PostMessage(msg);
            }
        }

        //----< button click handler >------------------------------------------------
        public void btnPublish_Click(object sender, RoutedEventArgs e)
        {
            String msg = "Publish";
            shim.PostMessage(msg);
        }

        //----< process message returned from the server and do relative action >------
        void ProcReceivedMsg(String msg)
        {
            string[] msgArray = msg.Split(new char[]{','});
            if (msgArray.Length == 0) return;

            string msgType = msgArray[0];

            if (msgType == "FileDirs")
            {
                string[] fileDirs = msgArray.ToList().GetRange(1, msgArray.Length - 1).ToArray();
                refreshRemoteListView(fileDirs);
                //statusBarItem.Content = "Status: Remote Refresh OK";
            }
            else if (msgType == "DelFile" || msgType == "DelDir")
            {
                statusBarItem.Content = "Status: " + msgArray[1];
                Thread.Sleep(100);
                btnRefreshRemote_Click(null, null);
            }
            else if (msgType == "File")
            {
                statusBarItem.Content = "Status: " + msgArray[1] + " downloaded";
            }
            else if (msgType == "Published")
            {
                statusBarItem.Content = "Status: All File Published";
                shim.PostMessage("DownloadCssJs");
                btnRefreshRemote_Click(null, null);
            }
            else
            {
                MessageBox.Show(msg, "ReceivedMsg", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        //----< start getting message after window loaded >------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            tRcv = new Thread(
                () =>
                {
                    while (true)
                    {
                        String msg = shim.GetMessage();
                        Action<String> act = new Action<string>(ProcReceivedMsg);
                        Object[] args = { msg };
                        Dispatcher.Invoke(act, args);
                    }
                }
            );
            tRcv.Start();

            btnPublish_Click(null, null);

            if (!doTest) return;
            // set current directory
            if (testPath == "" || !Directory.Exists(testPath)) return;
            Directory.SetCurrentDirectory(testPath);
            refreshLocalListView();

            tTest = new Thread(
                () =>
                {
                    Thread.Sleep(1000);
                    // do test executive
                    TestExecutive exec = new TestExecutive();
                    exec.DemoReq(this);
                }
            );
            tTest.Start(); 
        }

        //----< recursively get all files in the path >--------------------
        public List<string> recursiveGetFiles(string path)
        {
            List<string> fileList = new List<string>();
            
            string[] files = Directory.GetFiles(path, "*.*").Where(file => file.ToLower().EndsWith(".h") || file.ToLower().EndsWith(".cpp")).ToArray();
            foreach (string file in files)
                fileList.Add(file);

            string[] dirs = Directory.GetDirectories(path);
            foreach (string dir in dirs)
            {
                foreach (string file in recursiveGetFiles(dir))
                    fileList.Add(file);
            }

            return fileList;
        }
    }

    ///////////////////////////////////////////////////////////////////
    // FileDirItem class to store file informations for list view
    public class FileDirItem
    {
        public string Icon { get; set; }
        public string Name { get; set; }
        public string LastWriteTime { get; set; }
        public bool isFolder { get; set; }
    }
}
