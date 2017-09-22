using System;
using System.Windows;
using Microsoft.Win32;
using System.IO;
using ClearRoomLibrary;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Media.Imaging;
using System.Collections.Generic;

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
                //raw.Options.Threshold = 12.6f;
                //uint[] t = raw.Options.AverageAreaWhiteBalance;
                //t[0] = 10;
                //t[1] = 10;
                //t[2] = 100;
                //t[3] = 100;
                //raw.Options.AverageAreaWhiteBalance = t;
                //raw.Options.CameraWhiteBalance = true;
                //double[] ab = raw.Options.CorrectAberration;
                //raw.Options.Quality = 3;

                try
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
                        return;
                        byte[] buffer = raw.GetImage();

                        //image.Source = null;
                        //System.GC.Collect();
                        //GC.WaitForPendingFinalizers();

                        using (MemoryStream ms = new MemoryStream(buffer))
                        {
                            ////// Try creating a new image with a custom palette.
                            ////List<System.Windows.Media.Color> colors = new List<System.Windows.Media.Color>();
                            ////colors.Add(System.Windows.Media.Colors.Red);
                            ////colors.Add(System.Windows.Media.Colors.Blue);
                            ////colors.Add(System.Windows.Media.Colors.Green);
                            ////BitmapPalette myPalette = new BitmapPalette(colors);

                            /*
                            BitmapSource bitmapSource = BitmapSource.Create(5634, 3753, 600, 600, System.Windows.Media.PixelFormats.Rgb48, BitmapPalettes.Gray256, buffer, 5634 * 8);
                            bitmapSource.Freeze();
                            image.Source = bitmapSource;
                            */

                            System.Runtime.InteropServices.GCHandle pinnedArray = System.Runtime.InteropServices.GCHandle.Alloc(buffer, System.Runtime.InteropServices.GCHandleType.Pinned);
                            IntPtr pointer = pinnedArray.AddrOfPinnedObject();
                            // Do your stuff...
                            Bitmap b = new Bitmap(5634, 3753, 5634 * 8, PixelFormat.Format32bppRgb, pinnedArray.AddrOfPinnedObject());

                            System.Windows.Media.Imaging.BitmapSource bs = System.Windows.Interop.Imaging.CreateBitmapSourceFromHBitmap(b.GetHbitmap(), IntPtr.Zero, System.Windows.Int32Rect.Empty, BitmapSizeOptions.FromWidthAndHeight(b.Width, b.Height));
                            image.Source = bs;

                            b.Dispose();
                            pinnedArray.Free();

                            //BitmapImage bitmap = new BitmapImage();
                            //bitmap.BeginInit();
                            //bitmap.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
                            //bitmap.CacheOption = BitmapCacheOption.OnLoad;
                            ////    bitmap.DecodePixelHeight = 3753;
                            ////    bitmap.DecodePixelWidth= 5634;
                            ////    bitmap.UriSource = null;
                            //bitmap.StreamSource = ms;
                            ////////bitmap.StreamSource = ms;
                            ////////bitmap.CacheOption = BitmapCacheOption.OnLoad;
                            //bitmap.EndInit();
                            //bitmap.Freeze();
                            //    //// Assign the Source property of your image
                            //    image.Source = bitmap;// Source;
                            ////bitmapSource = null;
                            ////System.GC.Collect();
                            ////GC.WaitForPendingFinalizers();
                        }
                    }
                    //raw.Dispose();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                }
            }
        }

        // import this using System.Runtime.InteropServices;
        //private unsafe byte[] BmpToBytes_Unsafe(Bitmap bmp)
        //{
        //    BitmapData bData = bmp.LockBits(new Rectangle(new System.Drawing.Point(), bmp.Size),
        //        ImageLockMode.ReadOnly,
        //        PixelFormat.Format24bppRgb);
        //    // number of bytes in the bitmap
        //    int byteCount = bData.Stride * bmp.Height;
        //    byte[] bmpBytes = new byte[byteCount];

        //    // Copy the locked bytes from memory
        //    System.Runtime.InteropServices.Marshal.Copy(bData.Scan0, bmpBytes, 0, byteCount);

        //    // don't forget to unlock the bitmap!!
        //    bmp.UnlockBits(bData);

        //    return bmpBytes;
        //}


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
