using System.Runtime.InteropServices;

namespace SKT2
{

    public partial class Form1 : Form
    {
        private const string dllPath = @"C:\Users\Максим\source\repos\SKT2\x64\Debug\LibraryForSKT2.dll";

        [DllImport(dllPath)]
        private static extern void Initialize1(double I, int K, int n);

        [DllImport(dllPath)]
        private static extern void Initialize2(double alpha, double gamma);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern Point2D GetB(Point2D coordProfile, Elem elem);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern void CalcB([In, Out] Profile[] profile, int profileSize, Elem[] obj, int objSize);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern void CalcP([In, Out] double[] normP, Profile[] profile, int profileSize, [In, Out] Elem[] inverse_elems, int elemsSize);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern Elem GetElem(double[] X, double[] Z, double y, int i, int j);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetSplit(double start, double end, int N);

        private Panel drawingPanel1;
        private Panel drawingPanel2;

        double beginX;
        double beginZ;
        double endX;
        double endZ;
        int numX;
        int numZ;

        private Profile[] _profiles;
        private Elem[] _objects;
        private Elem[] _elements;
        private double I = 1.0;
        double[] _directNormP;
        private double[] _inverseNormP;

        private double Width;
        private double Height;

        private List<TextBox> _textBoxes = new();

        public Form1()
        {
            InitializeComponent();

            _textBoxes.Add(IntervalX1TextBox);
            _textBoxes.Add(IntervalX2TextBox);
            _textBoxes.Add(IntervalXNumTextBox);
            _textBoxes.Add(IntervalZ1TextBox);
            _textBoxes.Add(IntervalZ2TextBox);
            _textBoxes.Add(IntervalZNumTextBox);
            _textBoxes.Add(IntervalYTextBox);
            _textBoxes.Add(AlphaTextBox);
            _textBoxes.Add(GammaTextBox);
            _textBoxes.Add(ProfileX1TextBox);
            _textBoxes.Add(ProfileX2TextBox);
            _textBoxes.Add(ReceivNumTextBox);
            _textBoxes.Add(ProfileZTextBox);
            _textBoxes.Add(ObjectX1TextBox);
            _textBoxes.Add(ObjectX2TextBox);
            _textBoxes.Add(ObjectXNumTextBox);
            _textBoxes.Add(ObjectZ1TextBox);
            _textBoxes.Add(ObjectZ2TextBox);
            _textBoxes.Add(ObjectZNumTextBox);
            _textBoxes.Add(AlphaTextBox);
            _textBoxes.Add(GammaTextBox);
            _textBoxes.Add(PXtextBox);
            _textBoxes.Add(PZtextBox);

            drawingPanel1 = new Panel
            {
                Location = new Point(20, 300),
                Size = new Size(600, 300),
                BackColor = Color.White
            };
            drawingPanel2 = new Panel
            {
                Location = new Point(640, 300),
                Size = new Size(600, 300),
                BackColor = Color.White
            };

            Controls.Add(drawingPanel1);
            Controls.Add(drawingPanel2);

            InverseButton.Enabled = false;
        }

        private void Load_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog openFileDialog = new OpenFileDialog())
            {
                openFileDialog.InitialDirectory = "c:\\";
                openFileDialog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 2;
                openFileDialog.RestoreDirectory = true;

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {
                    string filePath = openFileDialog.FileName;

                    string[] lines = File.ReadAllLines(filePath);
                    for (int i = 0; i < lines.Length; i++)
                    {
                        _textBoxes[i].Text = lines[i];
                    }
                }
            }

            using (Graphics g = drawingPanel1.CreateGraphics())
            {
                g.FillRectangle(Brushes.White, 0, 0, drawingPanel1.Width, drawingPanel1.Height);
            }
            using (Graphics g = drawingPanel2.CreateGraphics())
            {
                g.FillRectangle(Brushes.White, 0, 0, drawingPanel2.Width, drawingPanel2.Height);
            }

            InverseButton.Enabled = false;
        }

        private void Save_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog saveFileDialog = new SaveFileDialog())
            {
                saveFileDialog.InitialDirectory = "c:\\";
                saveFileDialog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
                saveFileDialog.FilterIndex = 2;
                saveFileDialog.RestoreDirectory = true;

                if (saveFileDialog.ShowDialog() == DialogResult.OK)
                {
                    // Получаем путь к файлу для сохранения
                    string filePath = saveFileDialog.FileName;

                    // Собираем данные из текстбоксов
                    List<string> lines = new();
                    foreach (var textBox in _textBoxes)
                    {
                        lines.Add(textBox.Text);
                    }

                    // Сохраняем данные в файл
                    File.WriteAllLines(filePath, lines);
                }
            }
        }

        private void DirectButton_Click(object sender, EventArgs e)
        {
            Profile tmp = new();
            beginX = CheckDoubleValue(IntervalX1TextBox);
            double beginX1 = CheckDoubleValue(ProfileX1TextBox);
            double beginX2 = CheckDoubleValue(ObjectX1TextBox);
            beginZ = CheckDoubleValue(IntervalZ1TextBox);
            tmp.coord.z = CheckDoubleValue(ProfileZTextBox);
            double beginZ2 = CheckDoubleValue(ObjectZ1TextBox);
            endX = CheckDoubleValue(IntervalX2TextBox);
            double endX1 = CheckDoubleValue(ProfileX2TextBox);
            double endX2 = CheckDoubleValue(ObjectX2TextBox);
            endZ = CheckDoubleValue(IntervalZ2TextBox);
            double endZ2 = CheckDoubleValue(ObjectZ2TextBox);
            double y = CheckDoubleValue(IntervalYTextBox);
            numX = CheckIntValue(IntervalXNumTextBox);
            int numX1 = CheckIntValue(ReceivNumTextBox);
            int numX2 = CheckIntValue(ObjectXNumTextBox);
            numZ = CheckIntValue(IntervalZNumTextBox);
            int numZ2 = CheckIntValue(ObjectZNumTextBox);
            double pX = CheckDoubleValue(PXtextBox);
            double pZ = CheckDoubleValue(PZtextBox);

            IntPtr ptr = GetSplit(beginX, endX, numX);
            double[] X1 = new double[numX + 1];
            Marshal.Copy(ptr, X1, 0, X1.Length);

            ptr = GetSplit(beginZ, endZ, numZ);
            double[] Z1 = new double[numZ + 1];
            Marshal.Copy(ptr, Z1, 0, Z1.Length);

            ptr = GetSplit(beginX1, endX1, numX1 - 1);
            double[] grid = new double[numX1];
            Marshal.Copy(ptr, grid, 0, grid.Length);

            _profiles = new Profile[grid.Length];

            _elements = new Elem[numX * numZ];

            int K = _elements.Length;
            int n = _profiles.Length;

            Initialize1(I, K, n);

            int c = 0;
            for (int i = 0; i < Z1.Length - 1; i++)
            {
                for (int j = 0; j < X1.Length - 1; j++)
                {
                    _elements[c++] = GetElem(X1, Z1, y, i, j);
                }
            }

            for (int i = 0; i < _profiles.Length; i++)
            {
                tmp.coord.x = grid[i];
                _profiles[i] = tmp;
            }

            ptr = GetSplit(beginX2, endX2, numX2);
            double[] X2 = new double[numX2 + 1];
            Marshal.Copy(ptr, X2, 0, X2.Length);

            ptr = GetSplit(beginZ2, endZ2, numZ2);
            double[] Z2 = new double[numZ2 + 1];
            Marshal.Copy(ptr, Z2, 0, Z2.Length);

            _objects = new Elem[numX2 * numZ2];
            c = 0;
            for (int i = 0; i < Z2.Length - 1; i++)
            {
                for (int j = 0; j < X2.Length - 1; j++)
                {
                    _objects[c++] = GetElem(X2, Z2, y, i, j);
                }
            }
            for (int j = 0; j < _objects.Length; j++)
            {
                _objects[j].p.x = pX;
                _objects[j].p.z = pZ;
            }

            _directNormP = new double[_elements.Length];
            for (int i = 0; i < _objects.Length; i++)
            {
                for (int j = 0; j < _elements.Length; j++)
                {
                    if (_objects[i].end.x <= _elements[j].center.x && _elements[j].center.x <= _objects[i].begin.x &&
                        _objects[i].end.z <= _elements[j].center.z && _elements[j].center.z <= _objects[i].begin.z)
                    {
                        _directNormP[j] = NormP(_objects[i].p.x, _objects[i].p.z);
                    }
                }
            }

            CalcB(_profiles, _profiles.Length, _objects, _objects.Length);

            Width = (endX - beginX) / numX;
            Height = (endZ - beginZ) / numZ;

            drawingPanel1.Paint += DrawingPanel1_Paint;
            drawingPanel1.Invalidate();

            InverseButton.Enabled = true;

            double CheckDoubleValue(TextBox textBox)
            {
                if (double.TryParse(textBox.Text, out double value))
                {
                    return value;
                }
                else
                {
                    throw new Exception("Некорректное значение в TextBox");
                }
            }
            int CheckIntValue(TextBox textBox)
            {
                if (int.TryParse(textBox.Text, out int value))
                {
                    return value;
                }
                else
                {
                    throw new Exception("Некорректное значение в TextBox");
                }
            }

            double NormP(double px, double pz)
            {
                return Math.Sqrt(px * px + pz * pz);
            }
        }

        private void InverseButton_Click(object sender, EventArgs e)
        {
            double alpha = CheckDoubleValue(AlphaTextBox);
            double gamma = CheckDoubleValue(GammaTextBox);

            _inverseNormP = new double[_elements.Length];
            CalcP(_inverseNormP, _profiles, _profiles.Length, _elements, _elements.Length);

            drawingPanel2.Paint += DrawingPanel2_Paint;
            drawingPanel2.Invalidate();

            double CheckDoubleValue(TextBox textBox)
            {
                if (double.TryParse(textBox.Text, out double value))
                {
                    return value;
                }
                else
                {
                    throw new Exception("Некорректное значение в TextBox");
                }
            }
        }

        #region TextBox

        private void IntervalX1TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void IntervalX2TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void IntervalYTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void IntervalZ1TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void IntervalZ2TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void IntervalXNumTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void IntervalZNumTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void ProfileX1TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ProfileX2TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ReceivNumTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void ProfileZTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ObjectZ1TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ObjectXNumTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void ObjectX2TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ObjectX1TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void ObjectZNumTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForIntTextBox(sender, e);
        }
        private void ObjectZ2TextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void GammaTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void AlphaTextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void PXtextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }
        private void PZtextBox_KeyPress(object sender, KeyPressEventArgs e)
        {
            RulesForDoubleTextBox(sender, e);
        }

        private void RulesForDoubleTextBox(object sender, KeyPressEventArgs e)
        {
            // Разрешаем ввод только цифр, точки и минуса
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) && (e.KeyChar != ',') && (e.KeyChar != '-'))
            {
                e.Handled = true;
            }

            // Позволяем использование минуса только в начале строки
            if ((e.KeyChar == '-') && ((sender as TextBox).Text.IndexOf('-') > -1 || (sender as TextBox).SelectionStart != 0))
            {
                e.Handled = true;
            }

            // Разрешаем только одну точку
            if ((e.KeyChar == ',') && ((sender as TextBox).Text.IndexOf(',') > -1))
            {
                e.Handled = true;
            }
        }
        private void RulesForIntTextBox(object sender, KeyPressEventArgs e)
        {
            // Разрешаем ввод только цифр, точки и минуса
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar))
            {
                e.Handled = true;
            }
        }
        #endregion

        private void DrawingPanel1_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            double maxValue = _directNormP.Cast<double>().Max();
            int rectWidth = (drawingPanel1.Size.Width - 80) / numX;
            int rectHeight = (drawingPanel1.Size.Height - 40) / numZ;

            int c = 0;
            bool xDivisonScaleIsDrawned = false;

            for (int row = 0; row < numZ; row++)
            {
                int y = row * rectHeight + 10;

                for (int col = 0; col < numX; col++)
                {
                    int x = col * rectWidth + 40;

                    double normalizedValue = _directNormP[c++] / maxValue;
                    Color fillColor = GetGradientColor(normalizedValue);

                    using (Brush brush = new SolidBrush(fillColor))
                    {
                        g.FillRectangle(brush, x, y, rectWidth, rectHeight);
                    }
                    g.DrawRectangle(Pens.Black, x, y, rectWidth, rectHeight);

                    if (!xDivisonScaleIsDrawned)
                    {
                        string labelX = $"{col * Width + beginX,4}";
                        g.DrawString(labelX, Font, Brushes.Black, x - 10, numZ * rectHeight + 10);

                        if (col == numX - 1)
                        {
                            labelX = $"{numX * Width + beginX,4}";
                            x = numX * rectWidth + 40;
                            g.DrawString(labelX, Font, Brushes.Black, x - 10, numZ * rectHeight + 10);
                        }
                    }
                }

                string labelY = $"{-row * Height + endZ,4}";
                g.DrawString(labelY, Font, Brushes.Black, 0, y - 10);

                if (row == numZ - 1)
                {
                    labelY = $"{-numZ * Height + endZ,4}";
                    y = numZ * rectHeight + 10;
                    g.DrawString(labelY, Font, Brushes.Black, 0, y - 10);
                }

                xDivisonScaleIsDrawned = true;
            }
        }

        private void DrawingPanel2_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;

            double maxValue = _inverseNormP.Cast<double>().Max();
            int rectWidth = (drawingPanel2.Size.Width - 80) / numX;
            int rectHeight = (drawingPanel2.Size.Height - 40) / numZ;

            int c = 0;
            bool xDivisonScaleIsDrawned = false;

            for (int row = 0; row < numZ; row++)
            {
                int y = row * rectHeight + 10;

                for (int col = 0; col < numX; col++)
                {
                    int x = col * rectWidth + 40;

                    double normalizedValue = _inverseNormP[c++] / maxValue;
                    Color fillColor = GetGradientColor(normalizedValue);

                    using (Brush brush = new SolidBrush(fillColor))
                    {
                        g.FillRectangle(brush, x, y, rectWidth, rectHeight);
                    }
                    g.DrawRectangle(Pens.Black, x, y, rectWidth, rectHeight);

                    if (!xDivisonScaleIsDrawned)
                    {
                        string labelX = $"{col * Width + beginX,4}";
                        g.DrawString(labelX, Font, Brushes.Black, x - 10, numZ * rectHeight + 10);

                        if (col == numX - 1)
                        {
                            labelX = $"{numX * Width + beginX,4}";
                            x = numX * rectWidth + 40;
                            g.DrawString(labelX, Font, Brushes.Black, x - 10, numZ * rectHeight + 10);
                        }
                    }
                }

                string labelY = $"{-row * Height + endZ,4}";
                g.DrawString(labelY, Font, Brushes.Black, 0, y - 10);

                if (row == numZ - 1)
                {
                    labelY = $"{-numZ * Height + endZ,4}";
                    y = numZ * rectHeight + 10;
                    g.DrawString(labelY, Font, Brushes.Black, 0, y - 10);
                }

                xDivisonScaleIsDrawned = true;
            }
        }

        private Color GetGradientColor(double value)
        {
            // Цвета для интерполяции
            int r1 = 255, g1 = 255, b1 = 255; // Белый
            int r2 = 150, g2 = 75, b2 = 0;   // Коричневый

            // Линейная интерполяция цвета
            int r = (int)(r1 + value * (r2 - r1));
            int g = (int)(g1 + value * (g2 - g1));
            int b = (int)(b1 + value * (b2 - b1));

            return Color.FromArgb(r, g, b);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Point2D
    {
        public double x;
        public double z;

        public Point2D(double _x = 0, double _z = 0)
        {
            x = _x;
            z = _z;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Elem
    {
        public Point2D begin;
        public Point2D end;
        public Point2D center;
        public Point2D p;
        public Point2D gamma;
        public double mes;
        public double y;

        public Elem(double _mes = 0, double _y = 0)
        {
            begin = new Point2D();
            end = new Point2D();
            center = new Point2D();
            p = new Point2D();
            gamma = new Point2D();
            mes = _mes;
            y = _y;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Profile
    {
        public Point2D coord;
        public Point2D B;
    }
}