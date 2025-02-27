import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel 
from PyQt5.QtWidgets import QDesktopWidget, QMessageBox, QLineEdit, QProgressBar, QDial
from PyQt5.QtGui import QIcon, QPixmap
from PyQt5.QtCore import QSize, QTimer, Qt
from PyQt5.QtGui import QPainter, QColor, QFont
import datetime

# -------------------------------------------- DEFINED VARIABLES ------------------------------------------------ #

SERIAL_PORT = "COM10"
BAUD_RATE = 9600

PD6_state = 0
PD7_state = 0
PWM_percentage = 0
lcd_message = ' '

PB4_state = 0
PB3_state = 0
system_state = "OFFLINE"
temp = 0.0
pot1 = 0
pot2 = 0

class CircularLED(QWidget):
    COLOR_MAP = {
        "red": QColor(255, 0, 0),
        "green": QColor(0, 255, 0),
        "orange": QColor(255, 165, 0)
    }

    def __init__(self, window: QWidget, x, y, width, height, color="red", text="LED"):
        """Initialize the CircularLED inside the given window with fixed text."""
        super().__init__(window)  # Attach to the given QWidget (window)
        
        self.text = text  # Fixed text inside the LED
        self.border_color = QColor(0, 0, 0)  # Black border
        self.setGeometry(x, y, width, height)  # Set position and size

        # Set initial color (default to red if invalid)
        self.setColor(color)

    def setColor(self, color: str):
        """Change the LED color dynamically using predefined colors ('red', 'green', 'orange')."""
        if color.lower() in self.COLOR_MAP:
            self.color = self.COLOR_MAP[color.lower()]
            self.update()  # Repaint with new color
        else:
            print(f"Invalid color: {color}. Choose from 'red', 'green', or 'orange'.")

    def paintEvent(self, event):
        """Draw the circular LED with text."""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Draw the LED circle
        painter.setBrush(self.color)
        painter.setPen(self.border_color)  # Black border
        painter.drawEllipse(0, 0, self.width(), self.height())  # Draw full size

        # Draw the text inside the circle
        painter.setPen(QColor(0, 0, 0))  # Black text
        painter.setFont(QFont("Arial", 10, QFont.Bold))
        painter.drawText(self.rect(), Qt.AlignCenter, self.text)

        painter.end()

# ---------------------------------------------- COMMUNICATION -------------------------------------------------- #

def communication(ser, pot1_bar, pot2_bar, temp_bar, temp_label, PB4_led, PB3_led, Temp_led, Pot1_led, Pot2_led, System_label):
    global PD7_state, PD6_state, PWM_percentage, lcd_message

    # form the payload for the ntuaBoard
    message = f"msg:PD6:{PD6_state},PD7:{PD7_state},PWM:{PWM_percentage},LCD:{lcd_message}\n"

    # send payload
    send_serial_data(ser,message)

    # read received data from
    read_serial_data(ser, pot1_bar, pot2_bar, temp_bar, temp_label, PB4_led, PB3_led, Temp_led, Pot1_led, Pot2_led, System_label)


def send_serial_data(ser, message):
    if ser and ser.is_open:  # Ensure serial port is open
        ser.write(message.encode()) # encode() turns the message into bit-stream
        ser.flush()  # forces the program to wait until everything in the output buffer are send
        # print(f"Sent: {message}")
    else:
        print("Error: Serial port not open!")
    

def read_serial_data(ser, pot1_bar, pot2_bar, temp_bar, temp_label, PB4_led, PB3_led, Temp_led, Pot1_led, Pot2_led, System_label):
    if ser and ser.is_open and ser.in_waiting > 0:  # Check if data is available
        received_data = ser.readline()  # Read data until '\n'

        if received_data:
            received_data = received_data.decode(errors="replace").strip()
            print(f"Received :{received_data}")
            all_data = dict(pair.split(":") for pair in received_data.split(","))

            for key in all_data.keys():
                if(key == "POT1"):
                    pot1_bar_update(pot1_bar, all_data[key])
                elif(key == "POT2"):
                    pot2_bar_update(pot2_bar, all_data[key])
                elif(key == "TEMP"):
                    temp_bar_update(temp_bar, temp_label, all_data[key])
                elif(key == "BUTTONS"):
                    leds_update(PB4_led, PB3_led, all_data[key])
                elif(key == "STATUS"):
                    states_update(Temp_led, Pot1_led, Pot2_led, System_label, all_data[key])
        

# -------------------------------------------- UPDATE GUI FUNCTIONS ------------------------------------------- #

def pot1_bar_update(pot1_bar, value):
    global pot1
    pot1 = int(value)
    
    # Map potentiometer value (0-1023) to progress bar range (0-100)
    progress_value = int((pot1 / 1024) * 100)
    pot1_bar.setValue(progress_value)


def pot2_bar_update(pot2_bar, value):
    global pot2
    pot2 = int(value)
    
    # Map potentiometer value (0-1023) to progress bar range (0-100)
    progress_value = int((pot2 / 1024) * 100)         # !!! ADJUST THE SCALE ACCORDING TO ADC RESOLUTION !!!
    pot2_bar.setValue(progress_value)


def temp_bar_update(temp_bar, temp_label, value):
    global temp
    temp = float(value)
    temp_label.setText(f"Temperature: {temp}°C")

    # Update progress bar (convert float to int)
    value = int(temp)
    temp_bar.setValue(value)

    # Dynamic color change
    if value <= 25:
        temp_bar.setStyleSheet("QProgressBar::chunk { background-color: green; }")
    elif value <= 35:
        temp_bar.setStyleSheet("QProgressBar::chunk { background-color: orange; }")
    else:
        temp_bar.setStyleSheet("QProgressBar::chunk { background-color: red; }")


def leds_update(PB4_led, PB3_led, value):
    global PB4_state, PB3_state
    value = int(value)
    if(value == 0):
        PB4_led.setColor("red")
        PB3_led.setColor("red")
        PB4_state = 0
        PB3_state = 0
    elif(value == 1):
        PB4_led.setColor("red")
        PB3_led.setColor("green")
        PB4_state = 0
        PB3_state = 1  
    elif(value == 2):
        PB4_led.setColor("green")
        PB3_led.setColor("red")
        PB4_state = 1
        PB3_state = 0
    else:
        PB4_led.setColor("green")
        PB3_led.setColor("green")
        PB4_state = 1
        PB3_state = 1


def states_update(Temp_led, Pot1_led, Pot2_led, System_label, value):
    global system_state
    value = int(value)
    if( not(value & 0x01) ):
        System_label.setText("SYSTEM - OFFLINE")
        Temp_led.setColor("red")
        Pot1_led.setColor("red")
        Pot2_led.setColor("red")
        system_state = "OFFLINE"

    else:
        System_label.setText("SYSTEM - ONLINE")
        system_state = "ONLINE"
        # ---- Pot1_led ---- #
        if( not(value & 0b00000010) ):
            Pot1_led.setColor("red")
        elif( not(value & 0b00010000) ):
            Pot1_led.setColor("orange")
        else:
            Pot1_led.setColor("green")
        
        # ---- Pot2_led ---- #
        if( not(value & 0b00000100) ):
            Pot2_led.setColor("red")
        elif( not(value & 0b00100000) ):
            Pot2_led.setColor("orange")
        else:
            Pot2_led.setColor("green")
        
        # ---- Temp_led ---- #
        if( not(value & 0b00001000) ):
            Temp_led.setColor("red")
        elif( not(value & 0b01000000) ):
            Temp_led.setColor("orange")
        else:
            Temp_led.setColor("green")

# ----------------------------------------------- EVENT FUNCTIONS -------------------------------------------------------- #
def on_PD7_click(button):
    global PD7_state  # Declare it as global so we can modify it
    if(PD7_state == 0):
        PD7_state = 1
        button.setStyleSheet(f"background-color: green; color: black;")
    else:
        PD7_state = 0
        button.setStyleSheet(f"background-color: red; color: black;")


def on_PD6_click(button):
    global PD6_state  # Declare it as global so we can modify it
    if(PD6_state == 0): 
        PD6_state = 1
        button.setStyleSheet(f"background-color: green; color: black;")
    else:
        PD6_state = 0
        button.setStyleSheet(f"background-color: red; color: black;")


def change_PWM(dial_label, value):
    global PWM_percentage
    PWM_percentage = value
    dial_label.setText(f"Brightness: {value}%")


def save_message(lcd_text):
    global lcd_message
    lcd_message = lcd_text.text()


def time_stamp():
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    with open("LOG_file.txt", "a") as file:
        file.write(f"[{timestamp}] SYSTEM: {system_state}, PB3: {PB3_state}, PB4: {PB4_state}, PD6: {PD6_state}, PD7: {PD7_state}, ")
        file.write(f"TEMP: {temp}, POT1: {pot1}, POT2: {pot2}, PWM: {PWM_percentage}, LCD: {lcd_message}\n")

# ----------------------------------------------- GRAPHICS FUNCTIONS ----------------------------------------------- #
def window_position(window):
    screen = QDesktopWidget().screenGeometry()
    window_geometry = window.frameGeometry()
    x = (screen.width() - window_geometry.width()) // 2 
    y = (screen.height() - window_geometry.height()) // 2 - 70
    window.move(x, y)

# ------------------------------------------------------ MAIN -------------------------------------------------------- #
def main():

    ser = None
    # Try to Open Serial Connection
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1) #timeout=1 waits readline() for 1sec, if no data where read returns " "
    except serial.SerialException as e:
        print(f"Error: {e}")
        ser = None
    

    HMI = QApplication([])

    # Create the main window
    window = QWidget()
    window.setWindowTitle("SCADA NETWORK")
    window.resize(800, 500)
    window.setWindowIcon(QIcon("scada_logo.png"))
    #window.setStyleSheet("background-color: lightblue;")

    # Center the window
    window_position(window)

    # Add a label with a resized icon
    label = QLabel(window)  # Pass 'window' as the parent
    label_pixmap = QPixmap("scada_logo.png").scaled(64, 64)  # Resize to 64x64
    label.setPixmap(label_pixmap)
    label.move(10, 10)  # Position the label

    # ---- BUTTONS ---- #
    PD7 = QPushButton("PD7", window)
    PD7.setGeometry(580, 40, 70, 70)   # x, y, width, height
    PD7.setStyleSheet(f"background-color: red; color: black;")
    PD7.clicked.connect(lambda: on_PD7_click(PD7))

    PD6 = QPushButton("PD6", window)
    PD6.setGeometry(500, 40, 70, 70)
    PD6.setStyleSheet(f"background-color: red; color: black;")    
    PD6.clicked.connect(lambda: on_PD6_click(PD6))

    log_data = QPushButton("LOG", window)
    log_data.setGeometry(120, 380, 50, 50)   # x, y, width, height
    log_data.setStyleSheet(f"background-color: blue; color: black;")
    log_data.clicked.connect(lambda: time_stamp())

    # ---- POTENSIOMETERS PROGRESS BARS ---- #
    pot1_bar = QProgressBar(window)
    pot1_bar.setFormat("Potentiometer 1")  # Set static text
    pot1_bar.setGeometry(100, 40, 380, 30)
    pot1_bar.setMinimum(0)
    pot1_bar.setMaximum(100)  # Max value mapped to 100
    pot1_bar.setValue(0)  # Start at 0
    pot1_bar.setStyleSheet("""
        QProgressBar {
            border: 2px solid    #000000;
            border-radius: 5px; /* Round Corners*/
            text-align: center; /* Center text */
        }
        QProgressBar::chunk {
            background-color: #2196F3; /* Blue color for the bar */
        }
    """)

    pot2_bar = QProgressBar(window)
    pot2_bar.setFormat("Potentiometer 2")  # Set static text
    pot2_bar.setGeometry(100, 80, 380, 30)
    pot2_bar.setMinimum(0)
    pot2_bar.setMaximum(100)  # Max value mapped to 100
    pot2_bar.setValue(0)  # Start at 0
    pot2_bar.setStyleSheet("""
        QProgressBar {
            border: 2px solid    #000000;
            border-radius: 5px; /* Round Corners*/
            text-align: center; /* Center text */
        }
        QProgressBar::chunk {
            background-color: #2196F3; /* Blue color for the bar */
        }
    """)

    # ---- TEMPERATURES BAR ---- #
    temp_bar = QProgressBar(window)
    temp_bar.setTextVisible(False)
    temp_bar.setGeometry(695, 60, 60, 400)  # x, y, width, height (Tall for vertical)
    temp_bar.setOrientation(Qt.Vertical)  # Proper vertical orientation
    temp_bar.setMinimum(0)  # Min temperature (for safety, assume 0°C)
    temp_bar.setMaximum(50)  # Max temperature (safe upper limit)
    temp_bar.setValue(25)  # Start at 25
    temp_bar.setStyleSheet("QProgressBar::chunk { background-color: green; }")

    temp_label = QLabel("Temperature: 25.00°C", window)
    temp_label.setGeometry(660, 30, 150, 20)

    # ---- PWM DIAL ---- #
    dial = QDial(window)
    dial.setGeometry(490,130,100,100)
    dial.setMinimum(0)  # Minimum pressure
    dial.setMaximum(100)  # Maximum pressure
    dial.setValue(0)  # Initial value

    dial_label = QLabel("Brightness: 0%", window)
    dial_label.setGeometry(590, 170, 150, 20)

    dial.valueChanged.connect(lambda value: change_PWM(dial_label, value))

    # ---- LCD MESSAGE BUFFER ---- #
    lcd_text = QLineEdit(window)
    lcd_text.setGeometry(100, 130, 380, 50)
    lcd_text.setFont(QFont("Arial", 20))
    lcd_text.setAlignment(Qt.AlignCenter)
    lcd_text.setPlaceholderText("Message for LCD...")
    lcd_text.setMaxLength(16)

    save_button = QPushButton("Send Message", window)
    save_button.setGeometry(100, 190, 380, 30)
    save_button.clicked.connect(lambda: save_message(lcd_text))

    # ---- STATE LEDS ---- #
    PB3_led = CircularLED(window, x=120, y=280, width=60, height=60, color="red", text="PB3")
    PB4_led = CircularLED(window, x=230, y=280, width=60, height=60, color="red", text="PB4")
    Temp_led = CircularLED(window, x=340, y=280, width=60, height=60, color="red", text="TEMP")
    Pot1_led = CircularLED(window, x=450, y=280, width=60, height=60, color="red", text="POT1")
    Pot2_led = CircularLED(window, x=560, y=280, width=60, height=60, color="red", text="POT2")

    System_label = QLabel("SYSTEM - ONLINE", window)
    System_label.setGeometry(230, 230, 310, 40)  
    System_label.setFont(QFont("Arial", 20, QFont.Bold))

    # ---- QTimer INTERRUPT ---- #
    timer = QTimer()
    timer.timeout.connect(lambda: communication(ser, pot1_bar, pot2_bar, temp_bar, temp_label, PB4_led, PB3_led, Temp_led, Pot1_led, Pot2_led, System_label))
    timer.start(100)  # Start checking immediately

    # Close serial connection properly when app exits
    def close_serial():
        if ser and ser.is_open:
            ser.close()
            print("Serial port closed.")

    HMI.aboutToQuit.connect(close_serial)

    # Show the window
    window.show()

    # Run the application
    sys.exit(HMI.exec_())

if __name__ == "__main__":
    main()