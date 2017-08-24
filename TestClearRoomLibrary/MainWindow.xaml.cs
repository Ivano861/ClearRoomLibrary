using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using System.IO;
using ClearRoomLibrary;

namespace TestClearRoomLibrary
{
    /// <summary>
    /// Logica di interazione per MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private void SelectImageButton_Click(object sender, RoutedEventArgs e)
        {
            // Create an instance of the open file dialog box.
            OpenFileDialog openFileImage = new OpenFileDialog
            {
                // Set filter options and filter index.
                Filter = "Image Files (.jpg)|*.jpg|Image Tiff (.tiff)|*.tiff|All Files (*.*)|*.*",
                FilterIndex = 3,
                Multiselect = false
            };

            // Call the ShowDialog method to show the dialog box.
            bool? userClickedOK = openFileImage.ShowDialog();

            // Process input if the user clicked OK.
            if (userClickedOK == true)
            {
                FilenameSelected.Text = openFileImage.FileName;
            }
        }

        private void GetInfoButton_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(FilenameSelected.Text))
            {
                MessageBox.Show("Select iamge file to proceed.");
                return;
            }

            if (!File.Exists(FilenameSelected.Text))
            {
                MessageBox.Show("File selected not found.");
                return;
            }

            using (Raw raw = new Raw(FilenameSelected.Text))
            {
                using (SimpleInfo simple = raw.GetInfo())
                {

                }
                using (ImageLoader simple = raw.GetImageRaw())
                {

                }
                //raw.Dispose();
            }
        }
    }
}
