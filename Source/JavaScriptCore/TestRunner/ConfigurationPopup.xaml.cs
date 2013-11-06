using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using System.Windows.Controls.Primitives;

namespace TestRunner
{
    public partial class ConfigurationPopup : UserControl
    {
        public static bool annexB = true;
        public static bool bestPractice = true;
        public static bool ch06 = true;
        public static bool ch07 = true;
        public static bool ch08 = true;
        public static bool ch09 = true;
        public static bool ch10 = true;
        public static bool ch11 = true;
        public static bool ch12 = true;
        public static bool ch13 = true;
        public static bool ch14 = true;
        public static bool ch15 = true;
        public static bool intl402 = true;

        public ConfigurationPopup()
        {
            InitializeComponent();
            CheckannexB.IsChecked = annexB;
            CheckbestPractice.IsChecked = bestPractice;
            Checkch06.IsChecked = ch06;
            Checkch07.IsChecked = ch07;
            Checkch08.IsChecked = ch08;
            Checkch09.IsChecked = ch09;
            Checkch10.IsChecked = ch10;
            Checkch11.IsChecked = ch11;
            Checkch12.IsChecked = ch12;
            Checkch13.IsChecked = ch13;
            Checkch14.IsChecked = ch14;
            Checkch15.IsChecked = ch15;
            Checkintl402.IsChecked = intl402;
        }

        private void DoneCallback(object sender, RoutedEventArgs e)
        {
            annexB = CheckannexB.IsChecked == true;
            bestPractice = CheckbestPractice.IsChecked == true;
            ch06 = Checkch06.IsChecked == true;
            ch07 = Checkch07.IsChecked == true;
            ch08 = Checkch08.IsChecked == true;
            ch09 = Checkch09.IsChecked == true;
            ch10 = Checkch10.IsChecked == true;
            ch11 = Checkch11.IsChecked == true;
            ch12 = Checkch12.IsChecked == true;
            ch13 = Checkch13.IsChecked == true;
            ch14 = Checkch14.IsChecked == true;
            ch15 = Checkch15.IsChecked == true;
            intl402 = Checkintl402.IsChecked == true;

            Popup parent = this.Parent as Popup;
            parent.IsOpen = false;
        }
    }
}
