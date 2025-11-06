import serial
import time
import os

# === User Configuration ===
COM_PORT = 'COM3'      # Change if your Arduino uses a different COM port
BAUD_RATE = 9600

FILES = {
    "1": r"C:\Users\srini\Downloads\CD PROJECT\code_before_optimization.txt",
    "2": r"C:\Users\srini\Downloads\CD PROJECT\code_after_optimization.txt",
    "3": r"C:\Users\srini\Downloads\CD PROJECT\optimization_suggestions.txt"
}

# === Function: Send file contents to Arduino ===
def send_to_arduino(ser, file_path):
    """Reads a file and sends its text to Arduino."""
    if not os.path.exists(file_path):
        print(f"❌ File not found: {file_path}")
        return

    print(f"📤 Sending {os.path.basename(file_path)} ...")

    with open(file_path, 'r', encoding='utf-8') as file:
        text = file.read().replace('\n', ' ')

    # Limit message size (LCD cannot scroll very long text effectively)
    text = text[:500]  # Adjust length if needed

    ser.write(text.encode())
    ser.write(b'\n')
    ser.flush()

    print("✅ Sent successfully!\n")
    ser.reset_input_buffer()  # Clear input buffer for next keypad input


# === Function: Establish connection ===
def connect_serial():
    """Try to connect to the Arduino serial port."""
    while True:
        try:
            print(f"🔌 Connecting to Arduino on {COM_PORT} ...")
            ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            time.sleep(2)  # Allow time for Arduino to reset
            print("✅ Connected! Waiting for keypad input...\n")
            return ser
        except serial.SerialException:
            print("⚠️ Arduino not found. Retrying in 3 seconds...")
            time.sleep(3)


# === Main Loop ===
if __name__ == "__main__":
    ser = connect_serial()

    while True:
        try:
            if ser.in_waiting > 0:
                key = ser.readline().decode(errors='ignore').strip()
                if key in FILES:
                    send_to_arduino(ser, FILES[key])
                elif key != "":
                    print(f"⚠️ Unknown key received: {key}")

        except serial.SerialException:
            print("⚠️ Serial connection lost. Reconnecting...")
            time.sleep(2)
            ser.close()
            ser = connect_serial()

        except KeyboardInterrupt:
            print("\n🛑 Exiting program.")
            ser.close()
            break
