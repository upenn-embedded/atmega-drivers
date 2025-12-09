import serial

ser = serial.Serial('COM15', 115200)  # Change COM3 to your port
ser.write(b'Hello World!\n')
print("Message sent.")
ser.close()