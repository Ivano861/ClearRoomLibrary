using System.Windows;
using Microsoft.Win32;
using System.IO;
using ClearRoomLibrary;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Media.Imaging;

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
            //CreateBitmap();

            //return;

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
                    string make = simple.Make;
                    string model = simple.Model;
                    string model2 = simple.Model2;
                    string artist = simple.Artist;
                    string description = simple.Description;
                }
                using (ImageLoader simple = raw.GetImageRaw())
                {

                }
                //raw.Dispose();
            }
        }

        void CreateBitmap()
        {
            byte[] buffer = CreateGridImage(10, 10, 9, 9, 30);
            using (MemoryStream memoryStream = new MemoryStream(buffer))
            {
                var bitmap = new BitmapImage();
                bitmap.BeginInit();
                bitmap.StreamSource = memoryStream;
                bitmap.CacheOption = BitmapCacheOption.OnLoad;
                bitmap.EndInit();
                bitmap.Freeze();
                

                // Assign the Source property of your image
                image.Source = bitmap;
            }
            //ImageSource im = new DrawingImage();
            //System.Drawing.Bitmap flag = new System.Drawing.Bitmap(10, 10);
            //for (int x = 0; x < flag.Height; ++x)
            //    for (int y = 0; y < flag.Width; ++y)
            //        flag.SetPixel(x, y, System.Drawing.Color.White);
            //for (int x = 0; x < flag.Height; ++x)
            //    flag.SetPixel(x, x, System.Drawing.Color.Red);
            //image.Source = flag;
        }

        public static byte[] CreateGridImage(
            int maxXCells,
            int maxYCells,
            int cellXPosition,
            int cellYPosition,
            int boxSize)
        {
            using (var bmp = new Bitmap(maxXCells * boxSize + 1, maxYCells * boxSize + 1))
            {
                using (Graphics g = Graphics.FromImage(bmp))
                {
                    g.Clear(Color.Yellow);
                    Pen pen = new Pen(Color.Black)
                    {
                        Width = 1
                    };

                    //Draw red rectangle to go behind cross
                    Rectangle rect = new Rectangle(boxSize * (cellXPosition - 1), boxSize * (cellYPosition - 1), boxSize, boxSize);
                    g.FillRectangle(new SolidBrush(Color.Red), rect);

                    //Draw cross
                    g.DrawLine(pen, boxSize * (cellXPosition - 1), boxSize * (cellYPosition - 1), boxSize * cellXPosition, boxSize * cellYPosition);
                    g.DrawLine(pen, boxSize * (cellXPosition - 1), boxSize * cellYPosition, boxSize * cellXPosition, boxSize * (cellYPosition - 1));

                    //Draw horizontal lines
                    for (int i = 0; i <= maxXCells; i++)
                    {
                        g.DrawLine(pen, (i * boxSize), 0, i * boxSize, boxSize * maxYCells);
                    }

                    //Draw vertical lines            
                    for (int i = 0; i <= maxYCells; i++)
                    {
                        g.DrawLine(pen, 0, (i * boxSize), boxSize * maxXCells, i * boxSize);
                    }
                }

                var memStream = new MemoryStream();
                bmp.Save(memStream, ImageFormat.Bmp);
                return memStream.ToArray();
            }
        }
    }
}
