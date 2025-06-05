# Import necessary libraries
import serial
import time
import json
import tkinter as tk
from tkinter import messagebox

ser = None

def connect_arduino():
    global ser
    try:
        ser = serial.Serial("COM5", 9600, timeout=1)
        time.sleep(2)
        messagebox.showinfo("Connect", "Arduino connection is successful!")
    except Exception as e:
        messagebox.showerror("Error", f"Arduino connection is unsuccessful!: {e}")

from datetime import datetime, timedelta

def calculate_duration():
    # Calculate Duration (Hours) from Time Frame and update to period_entries.
    for i in range(8):
        try:
            time_range = time_entries[i].get().strip()  # Get input time frame
            start_time, end_time = time_range.split("-")  # Split into 2 parts
            start_time = datetime.strptime(start_time.strip(), "%H:%M")
            end_time = datetime.strptime(end_time.strip(), "%H:%M")

            # Past midnight cases
            if end_time < start_time:
                end_time += timedelta(days=1)   # Plus 24 hours

            # Calculate Duration (Hours) and round to 3 decimal places
            period_hours = (end_time - start_time).total_seconds() / 3600

            # Update to Duration (Hours)
            period_entries[i].delete(0, tk.END)
            period_entries[i].insert(0, str(period_hours))
        except Exception:
            period_entries[i].delete(0, tk.END)  # If error then clear input field

def calculate_blink_duration():
    # Calculate Duration (Hours) from the Time Frame in blink_period_entry.
    try:
        time_range = blink_period_entry.get().strip()  # Get input time frame
        start_time, end_time = time_range.split("-")  # Split into 2 parts
        start_time = datetime.strptime(start_time.strip(), "%H:%M")
        end_time = datetime.strptime(end_time.strip(), "%H:%M")

        # Past midnight cases
        if end_time < start_time:
            end_time += timedelta(days=1)  # Plus 24 hours

        # Calculate Duration (Hours) and round to 3 decimal places
        blink_hours = round((end_time - start_time).total_seconds() / 3600, 3)

        # Update to Duration (Hours)
        blink_period_hours_entry.delete(0, tk.END)
        blink_period_hours_entry.insert(0, str(blink_hours))
    except Exception:
        blink_period_hours_entry.delete(0, tk.END)  # If error then clear input field

# Calculate repeats using formula below
def calculate_repeats():
    for i in range(8):
        try:
            green_time = int(green_entries[i].get())
            yellow_time = int(yellow_entries[i].get())
            red_time = int(red_entries[i].get())
            period_hours = float(period_entries[i].get())

            total_cycle_time = (green_time + 1) + (yellow_time + 1) + (red_time + 1)
            total_period_seconds = period_hours * 3600
            repeats = round(total_period_seconds / total_cycle_time)
            calculated_repeat_entries[i].delete(0, tk.END)
            calculated_repeat_entries[i].insert(0, str(repeats))
        except ValueError:
            messagebox.showerror("Error", "Please enter valid numbers for all fields!")

        try:
            blink_period = float(blink_period_hours_entry.get())
            repeats_blink = round(blink_period * 1800)
            blink_repeat_calc_entry.delete(0, tk.END)
            blink_repeat_calc_entry.insert(0, str(repeats_blink))
        except ValueError:
            messagebox.showerror("Error", " Please enter valid numbers for all fields!")

# Calculate the deviation of the blinking yellow
def calculate_blink_deviation():
    try:
        blink_repeats = int(blink_repeat_entry.get())
        blink_period = float(blink_period_hours_entry.get())
        blink_deviation = (blink_repeats * 2) - (blink_period * 3600)
        return blink_deviation
    except ValueError:
        return 0

#Calculate total deviation to check the deviation between the calculation cycle and the real time
def calculate_total_deviation():
    total_deviation = 0
    for i in range(8):
        try:
            green_time = int(green_entries[i].get())
            yellow_time = int(yellow_entries[i].get())
            red_time = int(red_entries[i].get())
            repeats = int(manual_repeat_entries[i].get())
            period_hours = float(period_entries[i].get())

            total_cycle_time = (green_time + 1) + (yellow_time + 1) + (red_time + 1)
            total_period_seconds = period_hours * 3600
            total_deviation += total_cycle_time * repeats - total_period_seconds
        except ValueError:
            continue
    blink_deviation = calculate_blink_deviation()
    final_total_deviation = total_deviation + blink_deviation
    total_deviation_var.set(f"{round(final_total_deviation, 2)}")

# Send data to Arduino
def send_data():
    if ser and ser.is_open:
        try:
            data = ""
            for i in range(8):
                green_time = green_entries[i].get()
                yellow_time = yellow_entries[i].get()
                red_time = red_entries[i].get()
                repeat_count = manual_repeat_entries[i].get()
                data += f"{green_time},{yellow_time},{red_time},{repeat_count},"
            repeat_count_blink = blink_repeat_entry.get()
            blink_period = blink_period_entry.get()
            data += f"{repeat_count_blink},{blink_period}\n"
            ser.write(data.encode())
            print("Data submitted:", data.strip())  # Print in terminal

            # Wait for Arduino to respond and print to PyCharm terminal
            time.sleep(1)  # Give Arduino time to process
            while ser.in_waiting:
                response = ser.readline().decode(errors='ignore').strip()
                print("Arduino response:", response)
            messagebox.showinfo("Send data", "Data sent successfully!")
        except Exception as e:
            messagebox.showerror("Error", f"Unable to send data: {e}")
    else:
        messagebox.showerror("Error", "Arduino is not connected!")

def calculate_all_durations():    # For calculating all durations
    calculate_duration()
    calculate_blink_duration()
    calculate_total_deviation()

def save_data_to_file():          # Save data to file JSON
    try:
        data = {
            "time_entries": [entry.get() for entry in time_entries],
            "period_entries": [entry.get() for entry in period_entries],
            "green_entries": [entry.get() for entry in green_entries],
            "yellow_entries": [entry.get() for entry in yellow_entries],
            "red_entries": [entry.get() for entry in red_entries],
            "calculated_repeat_entries": [entry.get() for entry in calculated_repeat_entries],
            "manual_repeat_entries": [entry.get() for entry in manual_repeat_entries],
            "total_deviation": total_deviation_var.get(),
            "blink": {
                "blink_period_entry": blink_period_entry.get(),
                "blink_period_hours_entry": blink_period_hours_entry.get(),
                "blink_repeat_calc_entry": blink_repeat_calc_entry.get(),
                "blink_repeat_entry": blink_repeat_entry.get()
            }
        }
        with open("traffic_light_data.json", "w", encoding="utf-8") as f:
            json.dump(data, f, indent=4, ensure_ascii=False)
        messagebox.showinfo("Save data", "Data has been saved!")
    except Exception as e:
        messagebox.showerror("Error", f"Unable to save data: {e}")

def load_data_from_file():  # Load data from file
    try:
        with open("traffic_light_data.json", "r", encoding="utf-8") as f:
            data = json.load(f)
        for entries, values in [
            (time_entries, data.get("time_entries", [])),
            (period_entries, data.get("period_entries", [])),
            (green_entries, data.get("green_entries", [])),
            (yellow_entries, data.get("yellow_entries", [])),
            (red_entries, data.get("red_entries", [])),
            (calculated_repeat_entries, data.get("calculated_repeat_entries", [])),
            (manual_repeat_entries, data.get("manual_repeat_entries", []))
        ]:
            for entry, value in zip(entries, values):
                entry.delete(0, tk.END)
                entry.insert(0, value)

        blink_data = data.get("blink", {})
        blink_period_entry.delete(0, tk.END)
        blink_period_entry.insert(0, blink_data.get("blink_period_entry", ""))

        blink_period_hours_entry.delete(0, tk.END)
        blink_period_hours_entry.insert(0, blink_data.get("blink_period_hours_entry", ""))

        blink_repeat_calc_entry.delete(0, tk.END)
        blink_repeat_calc_entry.insert(0, blink_data.get("blink_repeat_calc_entry", ""))

        blink_repeat_entry.delete(0, tk.END)
        blink_repeat_entry.insert(0, blink_data.get("blink_repeat_entry", ""))

        total_deviation_var.set(data.get("total_deviation", ""))
    except FileNotFoundError:
        pass  # If there is no file, skip it.
    except Exception as e:
        messagebox.showerror("Error", f"Unable to load saved data: {e}")

def reset_fields():
    # Do not reset time_entries and blink_period_entry
    for entry_list in [period_entries, green_entries, yellow_entries, red_entries, calculated_repeat_entries, manual_repeat_entries]:
        for entry in entry_list:
            entry.delete(0, tk.END)
    blink_period_hours_entry.delete(0, tk.END)
    blink_repeat_calc_entry.delete(0, tk.END)
    blink_repeat_entry.delete(0, tk.END)
    total_deviation_var.set("")
    messagebox.showinfo("Reset data", "Data has been reset (except time frame).")

# Tkinter Interface
root = tk.Tk()
root.title("Traffic light controller")
root.resizable(True, True)

# Get screen size
screen_width = root.winfo_screenwidth()
screen_height = root.winfo_screenheight()

# Get window size
window_width = 1275
window_height = 650

# Set window position at the center or middle of the screen
position_x = (screen_width - window_width) // 2 - 5
position_y = (screen_height - window_height) // 2 - 35
root.geometry(f"{window_width}x{window_height}+{position_x}+{position_y}")
root.update_idletasks()

# Title and Serial Port
tk.Label(root, text="TRAFFIC LIGHT CONTROLLER", font=("Arial", 16, "bold")).pack(pady=10)

tk.Label(root, text="Serial Port: COM5", font=("Arial", 12)).pack()

frame_main = tk.Frame(root)
frame_main.pack(pady=10)

# Label and fields data
frames = [
    "5:00 - 6:30", "6:30 - 9:00", "09:00 - 11:30", "11:30 - 13:30",
    "13:30 - 16:00", "16:30 - 19:30", "19:30 - 22:00", "22:00 - 5:00"
]

columns = ["Time Frame", "Duration (Hours)", "Green (Seconds)", "Yellow (Seconds)", "Red (Seconds)", "Repeats (Calculated)", "Repeats (Manual Input)"]

tk.Label(frame_main, text="Time Frame", font=("Arial", 12)).grid(row=0, column=0)

time_entries = []
for i, frame in enumerate(frames):
    entry = tk.Entry(frame_main, width=12, font=("Arial", 12))
    entry.insert(0, frame)
    entry.grid(row=0, column=i+1, padx=10)
    time_entries.append(entry)

# Input fields include durations (hours), time for each light and number of repeats
green_entries, yellow_entries, red_entries, calculated_repeat_entries, manual_repeat_entries, period_entries = [], [], [], [], [], []
for row, column_name in enumerate(columns[1:]):
    tk.Label(frame_main, text=column_name, font=("Arial", 12)).grid(row=row+1, column=0, pady=5)
    for col in range(8):
        entry = tk.Entry(frame_main, width=12, font=("Arial", 12))
        entry.grid(row=row+1, column=col+1, padx=10)
        if row == 0:
            period_entries.append(entry)
        elif row == 1:
            green_entries.append(entry)
        elif row == 2:
            yellow_entries.append(entry)
        elif row == 3:
            red_entries.append(entry)
        elif row == 4:
            calculated_repeat_entries.append(entry)
        elif row == 5:
            manual_repeat_entries.append(entry)

# Blinking yellow night mode
frame_blink = tk.Frame(root)
frame_blink.pack(pady=10)

# Label (Move to left column)
tk.Label(frame_blink, text="Night mode (Blinking Yellow)", font=("Arial", 13, "bold")).grid(row=0, column=0, columnspan=2, sticky="w", pady=(0, 10))

# Right column: Step-by-step instructions (move up 1 line + space further apart)
instructions = [
    "Step 1: Adjust the time frame               Step 2: Select 'Calculate Time'",
    "Step 3: Enter the time for each green, yellow, red light",
    "Step 4: Select 'Calculate Repeats'",
    "Step 5: Re-enter the calculated repeats in 'Repeats (Manual Input)' (Adjustable for testing)",
    "Step 6: Select 'Calculate Deviation'",
    "Step 7: Adjust the Repeats (Manual Input) to minimize the deviation",
    "Step 8: Connect and send data to the Arduino (Save the results if needed)"
]

for i, step in enumerate(instructions, start=0):  # Start from row=0 instead of row=1
    tk.Label(frame_blink, text=step, font=("Arial", 11)).grid(row=i, column=4, padx=130, pady=5, sticky="w")  # Spacing by padx=160

# Left column: Input parameters including time frame, duration and repeats
tk.Label(frame_blink, text="Time Frame", font=("Arial", 12)).grid(row=1, column=0, sticky="w")
blink_period_entry = tk.Entry(frame_blink, width=12, font=("Arial", 12))
blink_period_entry.grid(row=1, column=1, padx=10, pady=7)
blink_period_entry.insert(0, "22:00 - 5:00")

tk.Label(frame_blink, text="Duration (Hours)", font=("Arial", 12)).grid(row=2, column=0, sticky="w")
blink_period_hours_entry = tk.Entry(frame_blink, width=12, font=("Arial", 12))
blink_period_hours_entry.grid(row=2, column=1, padx=10, pady=7)

tk.Label(frame_blink, text="Repeats (Calculated)", font=("Arial", 12)).grid(row=3, column=0, sticky="w")
blink_repeat_calc_entry = tk.Entry(frame_blink, width=12, font=("Arial", 12))
blink_repeat_calc_entry.grid(row=3, column=1, padx=10, pady=7)

tk.Label(frame_blink, text="Repeats (Manual Input)", font=("Arial", 12)).grid(row=4, column=0, sticky="w")
blink_repeat_entry = tk.Entry(frame_blink, width=12, font=("Arial", 12))
blink_repeat_entry.grid(row=4, column=1, padx=10, pady=7)

# Total deviation label and entry
total_deviation_var = tk.StringVar()
tk.Label(frame_blink, text="Total deviation (Seconds)", font=("Arial", 12, "bold")).grid(row=5, column=0, sticky="w")
total_deviation_entry = tk.Entry(frame_blink, textvariable=total_deviation_var, font=("Arial", 12), width=12, bg="white")
total_deviation_entry.grid(row=5, column=1, padx=10, pady=7)

# Function buttons
frame_buttons = tk.Frame(root)
frame_buttons.pack(pady=10)

tk.Button(frame_buttons, text="Calculate Time", font=("Arial", 13), command=calculate_all_durations).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Calculate Repeats", font=("Arial", 13), command=lambda: [calculate_repeats()]).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Calculate Deviation", font=("Arial", 13), command=calculate_total_deviation).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Connect", font=("Arial", 13), command=connect_arduino).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Send data", font=("Arial", 13), command=send_data).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Reset data", font=("Arial", 13), command=reset_fields).pack(side=tk.LEFT, padx=10)
tk.Button(frame_buttons, text="Save data", font=("Arial", 13), command=save_data_to_file).pack(side=tk.LEFT, padx=10)

load_data_from_file()

root.mainloop()
