using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Threading;
using System.IO;

namespace WpfGui
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            MainWindow wnd = new MainWindow();

            if (e.Args.Length > 0)
            {
                if (e.Args[0][0] != '/' && e.Args[0][0] != '*')
                    wnd.testPath = e.Args[0];
            }

            foreach (string arg in e.Args)
            {
                if (arg == "/t")
                    wnd.doTest = true;
            }
            
            wnd.Show();
        }
    }
}
