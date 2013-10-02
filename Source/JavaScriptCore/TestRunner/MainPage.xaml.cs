using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using TestRunner.Resources;
using Windows.Storage;
using System.Threading.Tasks;
using System.IO;
using TestRunnerComponent;
using System.Collections.ObjectModel;

namespace TestRunner
{
    public partial class MainPage : PhoneApplicationPage
    {
        TestRunnerComponent.ScriptRunner scriptRunner;
        bool runningTests = false;
        ObservableCollection<TestRunnerComponent.ScriptResult> failedTests = new ObservableCollection<TestRunnerComponent.ScriptResult>();

        public MainPage()
        {
            InitializeComponent();
            this.scriptRunner = new TestRunnerComponent.ScriptRunner();
            FailedTestsListSelector.ItemsSource = failedTests;
        }

        private static async Task<String> LoadTestHarnessScripts()
        {
            String cth = await LoadHarnessFile("cth.js");
            String sta = await LoadHarnessFile("sta.js");
            String ed = await LoadHarnessFile("ed.js");
            String testBuiltinObject = await LoadHarnessFile("testBuiltInObject.js");
            String testIntl = await LoadHarnessFile("testIntl.js");
            return cth + sta + ed + testBuiltinObject + testIntl;
        }

        private static async Task<String> LoadFileFromInstallationDirectorySubdirectory(string subdirectory, string filePath)
        {
            filePath = Windows.ApplicationModel.Package.Current.InstalledLocation.Path + "\\" + subdirectory + "\\" + filePath;
            StorageFile scriptFile = await StorageFile.GetFileFromPathAsync(filePath);
            Stream fileStream = await scriptFile.OpenStreamForReadAsync();
            using (StreamReader streamReader = new StreamReader(fileStream))
            {
                return streamReader.ReadToEnd();
            }
        }

        private static async Task<String> LoadSuiteFile(string filePath)
        {
            return await LoadFileFromInstallationDirectorySubdirectory("suite", filePath);
        }

        private static async Task<String> LoadHarnessFile(string filePath)
        {
            return await LoadFileFromInstallationDirectorySubdirectory("harness", filePath);
        }

        private async Task<String[]> GetTestPaths()
        {
            String testList = await LoadSuiteFile("script-files.txt");
            return testList.Split('\n').Where(t => t.Length > 0 && t[0] != '#').ToArray();
        }

        private async void StartTestsCallback(object sender, RoutedEventArgs e)
        {
            if (runningTests == true)
                return;

            runningTests = true;
            String[] testPaths = await GetTestPaths();
            String harnessString = await LoadTestHarnessScripts();

            StatusText.Text = "Found " + testPaths.Count() + " tests";
            Progress.Minimum = 0;
            Progress.Maximum = 200;
            Progress.Value = 0;
            failedTests.Clear();

            for (int i = 0; i < Progress.Maximum; i++)
            {
                String script = await LoadSuiteFile(testPaths[i]);
                String fullScript = harnessString + script;
                TestRunnerComponent.ScriptResult result = this.scriptRunner.RunScript(fullScript, testPaths[i]);

                StatusText.Text = "Ran " + (i + 1) + " of " + Progress.Maximum + " tests (" + failedTests.Count + " failed)";
                System.Diagnostics.Debug.WriteLine("Test " + testPaths[i] + "(" + i + ")" + " passed: " + result.Success());

                // FIXME: This likely deserves a better check.
                bool positiveTest = !script.Contains("@negative");
                if (positiveTest != result.Success())
                    failedTests.Add(result);

                Progress.Value = i;
            }
        }

        private void FailedTestClickedCallback(object sender, System.Windows.Input.GestureEventArgs e)
        {
            if (FailedTestsListSelector.SelectedItem is TestRunnerComponent.ScriptResult)
            {
                TestRunnerComponent.ScriptResult result = (TestRunnerComponent.ScriptResult)FailedTestsListSelector.SelectedItem;
                MessageBox.Show(result.TestPath() + "\n\n" + result.ExceptionString());
            }
        }
    }
}