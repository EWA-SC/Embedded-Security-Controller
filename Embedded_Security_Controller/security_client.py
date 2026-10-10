import serial
import threading
import time
import tkinter as tk
from tkinter import scrolledtext


# -----------------------------
# SERIAL CONFIGURATION
# -----------------------------

SERIAL_PORT = "COM4"     # Change this to your Arduino's COM port
BAUD_RATE = 9600


# -----------------------------
# CONNECT TO ARDUINO
# -----------------------------

arduino = serial.Serial(port=SERIAL_PORT, baudrate=BAUD_RATE, timeout=0.1)

time.sleep(2) # Arduino Uno usually resets when the serial port opens.


# -----------------------------
# GUI FUNCTIONS
# -----------------------------

def display_message(message):
    terminal.config(state="normal")

    terminal.insert(tk.END, message)

    terminal.see(tk.END)

    terminal.config(state="disabled")


def send_command(event=None):

    command = command_entry.get().strip()

    if command == "":
        return

    display_message("> " + command + "\n")

    arduino.write((command + "\n").encode())

    command_entry.delete(0, tk.END)


def read_serial():

    while True:

        try:

            if arduino.in_waiting > 0:

                response = arduino.readline().decode(errors="replace")

                if response:

                    root.after(0, display_message, response) # Tkinter GUI updates should happen through the main GUI thread.

        except serial.SerialException:

            root.after(0, display_message, "\n[SERIAL CONNECTION LOST]\n")
            break


def close_program():

    if arduino.is_open:
        arduino.close()

    root.destroy()


# -----------------------------
# CREATE WINDOW
# -----------------------------

root = tk.Tk()

root.title("Embedded Security Controller")

root.geometry("750x500")

root.configure(bg="black")


# -----------------------------
# TERMINAL OUTPUT
# -----------------------------

terminal = scrolledtext.ScrolledText(root, bg="black", fg="lime", insertbackground="white", font=("Consolas", 11), state="disabled")

terminal.pack(fill=tk.BOTH, expand=True, padx=10, pady=(10, 5))


# -----------------------------
# COMMAND INPUT
# -----------------------------

command_entry = tk.Entry(root, bg="black", fg="white", insertbackground="white", font=("Consolas", 11))

command_entry.pack(fill=tk.X, padx=10, pady=(5, 10))

command_entry.focus()

command_entry.bind("<Return>", send_command)


# -----------------------------
# SERIAL READER THREAD
# -----------------------------

serial_thread = threading.Thread(target=read_serial, daemon=True)

serial_thread.start()


# -----------------------------
# START MESSAGE
# -----------------------------

display_message("=== Embedded Security Controller Client ===\n")

display_message(f"Connected to {SERIAL_PORT} @ {BAUD_RATE} baud\n\n")

display_message("Enter Arduino commands below.\n\n")

root.protocol("WM_DELETE_WINDOW", close_program) # Handle window close button


# -----------------------------
# START GUI
# -----------------------------

root.mainloop()
