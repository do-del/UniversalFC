VERSION 5.00
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "mscomm32.ocx"
Begin VB.Form Form1 
   Caption         =   "电机控制"
   ClientHeight    =   6855
   ClientLeft      =   10005
   ClientTop       =   3600
   ClientWidth     =   9105
   LinkTopic       =   "Form1"
   ScaleHeight     =   6855
   ScaleWidth      =   9105
   Begin VB.Frame Frame5 
      Caption         =   "运动命令"
      Height          =   1695
      Left            =   1920
      TabIndex        =   25
      Top             =   5040
      Width           =   1695
      Begin VB.CommandButton BtnMotorRun 
         Caption         =   "PWM马达运转"
         Height          =   375
         Left            =   240
         TabIndex        =   27
         Top             =   1080
         Width           =   1215
      End
      Begin VB.CommandButton BtnStartUp 
         Caption         =   "马达启动"
         Height          =   375
         Left            =   240
         TabIndex        =   26
         Top             =   360
         Width           =   1215
      End
      Begin VB.Line Line4 
         BorderColor     =   &H80000010&
         X1              =   0
         X2              =   1680
         Y1              =   840
         Y2              =   840
      End
   End
   Begin VB.Frame Frame4 
      Caption         =   "互斥型操作"
      Height          =   1455
      Left            =   1920
      TabIndex        =   17
      Top             =   3480
      Width           =   1695
      Begin VB.OptionButton OpPin 
         Caption         =   "C-(互斥型)开"
         Height          =   255
         Index           =   2
         Left            =   120
         TabIndex        =   20
         Top             =   1080
         Width           =   1455
      End
      Begin VB.OptionButton OpPin 
         Caption         =   "B-(互斥型)开"
         Height          =   255
         Index           =   1
         Left            =   120
         TabIndex        =   19
         Top             =   720
         Width           =   1455
      End
      Begin VB.OptionButton OpPin 
         Caption         =   "A-(互斥型)开"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   18
         Top             =   360
         Width           =   1455
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "PWM设置"
      Height          =   3255
      Left            =   1920
      TabIndex        =   3
      Top             =   120
      Width           =   1695
      Begin VB.CommandButton BtnPWMOff 
         Caption         =   "6路总关断"
         Height          =   375
         Index           =   4
         Left            =   120
         TabIndex        =   24
         Top             =   2760
         Width           =   1455
      End
      Begin VB.OptionButton OpPWM 
         Caption         =   "PWM_C_开"
         Height          =   375
         Index           =   2
         Left            =   120
         TabIndex        =   23
         Top             =   1680
         Width           =   1095
      End
      Begin VB.OptionButton OpPWM 
         Caption         =   "PWM_B_开"
         Height          =   375
         Index           =   1
         Left            =   120
         TabIndex        =   22
         Top             =   1320
         Width           =   1095
      End
      Begin VB.OptionButton OpPWM 
         Caption         =   "PWM_A_开"
         Height          =   375
         Index           =   0
         Left            =   120
         TabIndex        =   21
         Top             =   960
         Width           =   1095
      End
      Begin VB.CommandButton BtnPWMOff 
         Caption         =   "3路PWM全关"
         Height          =   375
         Index           =   3
         Left            =   120
         TabIndex        =   16
         Top             =   2160
         Width           =   1455
      End
      Begin VB.CommandButton BtnPWMSet 
         Caption         =   "PWM值"
         Height          =   255
         Left            =   840
         TabIndex        =   15
         Top             =   360
         Width           =   735
      End
      Begin VB.TextBox TxtPWMValue 
         Height          =   270
         Left            =   120
         TabIndex        =   14
         Text            =   "0"
         Top             =   360
         Width           =   615
      End
      Begin VB.Line Line3 
         BorderColor     =   &H80000010&
         X1              =   0
         X2              =   1680
         Y1              =   840
         Y2              =   840
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "端口电平"
      Height          =   5055
      Left            =   120
      TabIndex        =   2
      Top             =   1680
      Width           =   1575
      Begin VB.CommandButton BtnPinOff 
         Caption         =   "普通全关"
         Height          =   375
         Index           =   2
         Left            =   240
         TabIndex        =   12
         Top             =   4440
         Width           =   1095
      End
      Begin VB.CommandButton BtnPinOff 
         Caption         =   "A-B-C-全关"
         Height          =   375
         Index           =   1
         Left            =   240
         TabIndex        =   11
         Top             =   3960
         Width           =   1095
      End
      Begin VB.CommandButton BtnPinOff 
         Caption         =   "A+B+C+全关"
         Height          =   375
         Index           =   0
         Left            =   240
         TabIndex        =   10
         Top             =   3480
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "C-开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   5
         Left            =   240
         TabIndex        =   9
         Top             =   2880
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "B-开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   4
         Left            =   240
         TabIndex        =   8
         Top             =   2400
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "A-开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   3
         Left            =   240
         TabIndex        =   7
         Top             =   1920
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "C+开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   2
         Left            =   240
         TabIndex        =   6
         Top             =   1320
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "B+开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   1
         Left            =   240
         TabIndex        =   5
         Top             =   840
         Width           =   1095
      End
      Begin VB.CommandButton BtnPin 
         Caption         =   "A+开"
         Enabled         =   0   'False
         Height          =   375
         Index           =   0
         Left            =   240
         TabIndex        =   4
         Top             =   360
         Width           =   1095
      End
      Begin VB.Line Line2 
         BorderColor     =   &H80000010&
         X1              =   0
         X2              =   1560
         Y1              =   3360
         Y2              =   3360
      End
      Begin VB.Line Line1 
         BorderColor     =   &H80000010&
         X1              =   0
         X2              =   1560
         Y1              =   1800
         Y2              =   1800
      End
   End
   Begin MSCommLib.MSComm MSComm1 
      Left            =   5040
      Top             =   5880
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      CommPort        =   2
      DTREnable       =   -1  'True
      BaudRate        =   57600
      InputMode       =   1
   End
   Begin VB.Frame Frame1 
      Caption         =   "信号灯"
      Height          =   1455
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   1575
      Begin VB.CheckBox CkLed 
         Caption         =   "绿LED"
         ForeColor       =   &H0000C000&
         Height          =   255
         Index           =   1
         Left            =   240
         TabIndex        =   13
         Top             =   960
         Width           =   1095
      End
      Begin VB.CheckBox CkLed 
         Caption         =   "红LED"
         ForeColor       =   &H000000FF&
         Height          =   255
         Index           =   0
         Left            =   240
         TabIndex        =   1
         Top             =   360
         Width           =   1095
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Const MOTORADDR = 1









                                                '窗口启动
Private Sub Form_Load()
    If MSComm1.PortOpen = False Then MSComm1.PortOpen = True
    
End Sub
                                                'LED灯控制
Private Sub CkLed_Click(Index As Integer)
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    
    If Index = 0 Then cmd(1) = 1
    If Index = 1 Then cmd(1) = 2
    
    If CkLed(Index).Value = vbChecked Then cmd(3) = 1
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub
                                                '(Radio)互斥型A-B-C-开控制
Private Sub OpPin_Click(Index As Integer)
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    
    If Index = 0 Then cmd(1) = 6
    If Index = 1 Then cmd(1) = 7
    If Index = 2 Then cmd(1) = 8
    
    If OpPin(Index).Value = True Then cmd(3) = 2
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub
                                                'Radio-3路PWM开控制
Private Sub OpPWM_Click(Index As Integer)
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    
    If Index = 0 Then cmd(1) = 3
    If Index = 1 Then cmd(1) = 4
    If Index = 2 Then cmd(1) = 5
    
    If OpPWM(Index).Value = True Then cmd(3) = 2
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub
                                                '设置PWM值
Private Sub BtnPWMSet_Click()
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    cmd(1) = 10
    
    cmd(3) = CInt(TxtPWMValue.Text) Mod 256
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub
                                                'Btn关断MOSFEET按钮
Private Sub BtnPWMOff_Click(Index As Integer)
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    cmd(1) = 9
    
    cmd(3) = Index
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
    
    Dim i As Integer
    For i = 0 To 2
        If Index = 3 Or Index = 4 Then OpPWM(i).Value = False
        If Index = 4 Then OpPin(i).Value = False
    Next i
End Sub
                                                '马达启动命令
Private Sub BtnStartUp_Click()
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    cmd(1) = 11
    
    cmd(3) = CInt(TxtPWMValue.Text) Mod 256
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub
                                                'PWM马达运转命令
Private Sub BtnMotorRun_Click()
    Dim cmd(5) As Byte
    cmd(0) = MOTORADDR
    cmd(1) = 12
    
    cmd(3) = CInt(TxtPWMValue.Text) Mod 256
    
    Call CalCrc(cmd)
    Call MscommSend(cmd)
End Sub





                                                '串口发送例程
Private Sub MscommSend(send() As Byte)
    Dim a As Variant
    a = MSComm1.Input
    a = send
    MSComm1.Output = a
End Sub
                                                '计算CRC校验码
Public Sub CalCrc(dat() As Byte)
    Dim i As Integer, arylen As Integer
    Dim crcsum As Long
    
    arylen = UBound(dat)
    For i = 0 To arylen - 2
        crcsum = crcsum + dat(i)
    Next i
    
    dat(arylen - 1) = crcsum \ 256
    dat(arylen - 0) = crcsum Mod 256
End Sub
