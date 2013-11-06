using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Storage;
using System.Threading.Tasks;
using TestRunnerComponent;
using System.Collections.ObjectModel;
using Windows.UI.Popups;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace TestRunner
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        TestRunnerComponent.ScriptRunner scriptRunner;
        String[] allTestPaths;
        String[] filteredTestPaths;
        String harnessString;
        ObservableCollection<TestRunnerComponent.ScriptResult> failedTests = new ObservableCollection<TestRunnerComponent.ScriptResult>();


        public MainPage()
        {
            this.InitializeComponent();
            this.scriptRunner = new TestRunnerComponent.ScriptRunner();
            FailedTestsListSelector.ItemsSource = failedTests;

            StartTestsButton.IsEnabled = false;
            ConfigureButton.IsEnabled = false;

            StatusText.Text = "Loading tests...";
        }

        private async void PageLoadedCallback(object sender, RoutedEventArgs e)
        {
            allTestPaths = await GetTestPaths();
            harnessString = await LoadTestHarnessScripts();
            FilterTestPaths();

            Progress.Minimum = 0;
            Progress.Value = 0;
            StartTestsButton.IsEnabled = true;
            ConfigureButton.IsEnabled = true;
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }

        private void FilterTestPaths()
        {
           filteredTestPaths = allTestPaths.Where(testPath => 
                        (testPath.StartsWith("annexB") && ConfigurationDialog.annexB)
                     || (testPath.StartsWith("bestPractice") && ConfigurationDialog.bestPractice)
                     || (testPath.StartsWith("ch06") && ConfigurationDialog.ch06)
                     || (testPath.StartsWith("ch07") && ConfigurationDialog.ch07)
                     || (testPath.StartsWith("ch08") && ConfigurationDialog.ch08)
                     || (testPath.StartsWith("ch09") && ConfigurationDialog.ch09)
                     || (testPath.StartsWith("ch10") && ConfigurationDialog.ch10)
                     || (testPath.StartsWith("ch11") && ConfigurationDialog.ch11)
                     || (testPath.StartsWith("ch12") && ConfigurationDialog.ch12)
                     || (testPath.StartsWith("ch13") && ConfigurationDialog.ch13)
                     || (testPath.StartsWith("ch14") && ConfigurationDialog.ch14)
                     || (testPath.StartsWith("ch15") && ConfigurationDialog.ch15)
                     || (testPath.StartsWith("intl402") && ConfigurationDialog.intl402)).ToArray();

           StatusText.Text = "Found " + filteredTestPaths.Count() + " tests";
           Progress.Maximum = filteredTestPaths.Count();
        }

        private static async Task<String> LoadTestHarnessScripts()
        {
            String cth = await LoadHarnessFile("cth.js");
            String sta = await LoadHarnessFile("sta.js");
            String ed = await LoadHarnessFile("ed.js");
            String testBuiltinObject = await LoadHarnessFile("testBuiltInObject.js");
            String testIntl = await LoadHarnessFile("testIntl.js");
            return cth + sta + ed + testBuiltinObject + testIntl + "\n";
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

        static String StrictModeString(String testContents)
        {
            if (testContents.Contains("@onlyStrict"))
                return "\"use strict\";\nvar strict_mode = true;\n";
            else
                return "var strict_mode = false; \n";
        }


        private async void StartTestsCallback(object sender, RoutedEventArgs e)
        {
            this.StartTestsButton.IsEnabled = false;
            this.ConfigureButton.IsEnabled = false;

            this.failedTests.Clear();
            for (int i = 0; i < Progress.Maximum; i++)
            {
                String script = await LoadSuiteFile(filteredTestPaths[i]);
                bool positiveTest = !script.Contains("@negative");

                String fullScript = StrictModeString(script) + harnessString + script;
                TestRunnerComponent.ScriptResult result = this.scriptRunner.RunScript(fullScript, this.filteredTestPaths[i]);

                StatusText.Text = "Ran " + (i + 1) + " of " + Progress.Maximum + " tests (" + this.failedTests.Count + " failed)";
                System.Diagnostics.Debug.WriteLine("Test " + this.filteredTestPaths[i] + "(" + i + ")" + " passed: " + result.Success() + " should be: " + positiveTest);

                // FIXME: This likely deserves a better check.
                if (positiveTest != result.Success())
                    this.failedTests.Add(result);

                Progress.Value = i;
            }

            this.StartTestsButton.IsEnabled = true;
            this.ConfigureButton.IsEnabled = true;
        }

        private void ConfigureCallback(object sender, RoutedEventArgs e)
        {
            Popup configurationPopup = new Popup();
            configurationPopup.Child = new ConfigurationDialog();
            configurationPopup.IsOpen = true;

            configurationPopup.Closed += (s1, e1) =>
            {
                FilterTestPaths();
            };
        }
         
        
        private async void FailedTestClickedCallback(object sender,TappedRoutedEventArgs e)
        {
            if (FailedTestsListSelector.SelectedItem is TestRunnerComponent.ScriptResult)
            {
                TestRunnerComponent.ScriptResult result = (TestRunnerComponent.ScriptResult)FailedTestsListSelector.SelectedItem;             
                await new MessageDialog(result.ExceptionString()).ShowAsync();
            }
        }
    }
}
