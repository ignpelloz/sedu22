using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ardilla
{
    public partial class Form1 : Form
    {
        public int i;
        

        public Form1()
        {
            InitializeComponent();
            i = 0;
            serialPort1.Open();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            textBox1.Text = "Boton pulsado!: " + i;
            i++;
            if(i == 4){
                i = 0;
            }

            TxCmd('L', (byte) i);
        }

        public void TxCmd(char cmd, byte value) {
            byte[] buffer = new Byte[5];

            buffer[0] = 0x7E;
            buffer[1] = 2;
            buffer[2] = (byte)cmd;
            buffer[3] = value;
            buffer[4] = 0x0D;

            serialPort1.Write(buffer, 0, 5);
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

        public int RxStatus, RxLen, RxIdx;
        public int [] RxBuff;

        public void rxByteStateMachine(int RxByte)
        {
            switch(RxStatus){
                case 0:
                    if(RxByte == 0x7E){
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
                    if(RxIdx == RxLen){
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
            
            x = RxBuff[0] * 256 + RxBuff[1];
            y = RxBuff[2] * 256 + RxBuff[3];
            b = RxBuff[4];

            
            this.Invalidate();
        }

            
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            serialPort1.Close();
        }

        private void Form1_Paint(object sender, PaintEventArgs e)
        {
            textBox1.Text = "Eje x: " + x + " Eje y: " + y + "Bot: " + b;
            progressBar1.Value = x/16;
            progressBar2.Value = y/16;
            
            switch(b){
                case 0:
                    
                    break;
                case 1:
                    panel1.BackColor = Color.Yellow;
                    break;
                case 2:
                    panel1.BackColor = Color.Green;
                    break;
                case 3:
                    panel1.BackColor = Color.Orange;
                    break;
                case 4:
                    panel1.BackColor = Color.Violet;
                    break;
                case 5:
                    panel1.BackColor = Color.Blue;
                    break;
                default:
                    panel1.BackColor = Color.White;
                    break;

            }

        }

        private void trackBar1_Scroll(object sender, EventArgs e)
        {
            TxCmd('S', (byte)trackBar1.Value);
            label1.Text = "Servo: " + trackBar1.Value;
        }

        private void trackBar2_Scroll(object sender, EventArgs e)
        {
            TxCmd('M', (byte)trackBar2.Value);
            label2.Text = "Motor: " + trackBar2.Value;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            trackBar1.Value = 90;
            trackBar1_Scroll(sender, e);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            trackBar2.Value = 0;
            trackBar2_Scroll(sender, e);
        }
    }
}
