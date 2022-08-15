using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace WindowsFormsApplication1
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            serialPort1.Open();
        }

        private void boton_pulsado(object sender, EventArgs e)
        {
            textBoxMensajes.Text = "Botón pulsado!";
        }

        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            int b;
            while (serialPort1.BytesToRead > 0)
            {
                b = serialPort1.ReadByte();
                rxByteStateMachine(b);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            serialPort1.Close();
        }

        public int RxStatus, RxLen, RxIdx;
        public int[] RxBuff;

        public void rxByteStateMachine(int RxByte)
        {
            switch (RxStatus)
            {
                case 0:
                    if (RxByte == 0x7E)
                    {
                        RxStatus = 1;
                    }
                    break;
                case 1:
                    RxLen = RxByte;
                    RxIdx = 0;
                    RxBuff = new int[RxLen];
                    RxStatus = 2;
                    break;

                case 2:
                    RxBuff[RxIdx] = RxByte;
                    RxIdx++;
                    if (RxIdx == RxLen)
                    {
                        RxStatus = 3;
                    }
                    break;

                case 3:
                    if (RxByte == 0x0D)
                    {
                        ParsePacket(RxBuff);
                    }
                    RxStatus = 0;
                    break;

                default:
                    break;

            }
        }

        int x, y, b;
        public void ParsePacket(int[] RxBuff)
        {

            x = (RxBuff[0] << 8) | RxBuff[1];
            y = (RxBuff[2] << 8) | RxBuff[3];
            b = RxBuff[4];


            this.Invalidate();
        }

        private void Form1_Paint(object sender, PaintEventArgs e)
        {
            textBoxMensajes.Text = "Eje x: " + x + " Eje y: " + y + "Bot: " + b;
            progressBar1.Value = x;
            progressBar2.Value = y;

            switch (b)
            {
                case 0:
                    panel1.BackColor = Color.Red;
                    break;
                case 1:
                    panel1.BackColor = Color.White;
                    break;
                case 2:
                    panel1.BackColor = Color.Blue;
                    break;
                case 3:
                    panel1.BackColor = Color.Yellow;
                    break;
                case 4:
                    panel1.BackColor = Color.Green;
                    break;
                case 5:
                    panel1.BackColor = Color.Turquoise;
                    break;
            }
            
        }
    }
}
