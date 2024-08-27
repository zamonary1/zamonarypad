import dearpygui.dearpygui as dpg
import pymsgbox
import serial
import serial.tools.list_ports
import time
import json
import scipy.interpolate

# global variables
ser = None
lasttime = time.time()
json_response = {"button1val":0,"button2val":0,"button1sens":0,"button2sens":0,"button1val_raw":0,"button2val_raw":0}
json_response_raw = json_response
min1 = float('inf')
min2 = float('inf')
max1 = float('-inf')
max2 = float('-inf')
delay_between_reads = 0.033 #33ms between updates or 30 updates per second
# get availible serial interfaces
def get_serial_ports():
    ports = serial.tools.list_ports.comports()
    ports = [port.device for port in ports]
    ports = [x for x in ports if 'ttyACM' in x] #keep only elements containing ttyACM
    if ports == []: 
        try: 
            dpg.set_value("connection_status", "Device not found")
            dpg.hide_item(serial_devices_list)
        except SystemError: next
    else:
        try: 
            dpg.set_value("connection_status", f"Found {len(ports)} devices")
            dpg.show_item(serial_devices_list)
        except SystemError: next
    return ports

# create serial connection
def connect_serial():
    global ser
    port = dpg.get_value("serial_port")
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        dpg.set_value("connection_status", f"Connected to {port}")
        lasttime = time.time()
    except serial.SerialException as err:
        print(err)
        ser = None
        if 'Device or resource busy' in str(err):
            dpg.set_value("connection_status", "Failed to connect:\nDevice or resource busy")
        else: dpg.set_value("connection_status", "Failed to connect")

def refresh_serial():
    dpg.configure_item("serial_port", items=get_serial_ports())
    
def lostConnection():
    ser = None
    refresh_serial()
    dpg.set_value("connection_status", "Lost connection")
    dpg.set_value("progress_bar_1", 0.0)
    dpg.set_value("progress_bar_2", 0.0)
    json_response = json.loads('{"button1val":0,"button2val":0}') #reset button statusbars to 0

def readbuttonvalues(raw = False):
    global lasttime
    global json_response
    global min1
    global min2
    global max1
    global max2
    global ser
    
    if lasttime + delay_between_reads < time.time() and ser != None:
        try: ser.write(b'read\n')
        except Exception:
            if ser != None:
                lostConnection()
                return 0
        # ser.write(b'read\n')

        try: json_response = json.loads(ser.readline())
        except serial.serialutil.SerialException:
            lostConnection()
            return 0
        #print(json_response)
        
        min1 = min(min1, int(json_response['button1val']))
        min2 = min(min2, int(json_response['button2val']))
        max1 = max(max1, int(json_response['button1val']))
        max2 = max(max2, int(json_response['button2val']))
        
        json_response['button1val_raw'] = json_response['button1val']
        json_response['button2val_raw'] = json_response['button2val']
        
        if max1 - min1 != 0:
            json_response['button1val'] = (int(json_response['button1val']) - min1) / (max1 - min1) * 100
        else:
            json_response['button1val'] = 0

        if max2 - min2 != 0:
            json_response['button2val'] = (int(json_response['button2val']) - min2) / (max2 - min2) * 100
        else:
            json_response['button2val'] = 0
        
        lasttime += delay_between_reads
    
    if raw: 
        ser.write(b'read\n')
        return(json.loads(ser.readline()))
    
    return(json_response)

def send_sens_values(btn1_sens, btn2_sens):
    if ser is not None:
        json_response = readbuttonvalues()
        val_btn1_idle = int(json_response["button1val_raw"])
        val_btn2_idle = int(json_response["button2val_raw"])
        print(f"sens1 = {val_btn1_idle + btn1_sens}, sens2 = {val_btn2_idle + btn2_sens}")
        time.sleep(0.1)
        ser.write(f'wrbtn1 {val_btn1_idle + btn1_sens}\n'.encode())
        time.sleep(0.1)
        ser.write(f'wrbtn2 {val_btn2_idle + btn2_sens}\n'.encode())
        print(ser.readline())
        return(True)
    else: return(False)

def calibrate_btn():
    #Data was measured by me
    x_grams =    [0.25, 0.45,  1.25,  1.6,   3.9,   9.5,   14,    29.6 ]
    y_measured = [6000, 10000, 15000, 20000, 35000, 50000, 60000, 70000]

    if ser is not None:
        
        #pymsgbox.alert(text="Do not touch the zamonaryboard and press OK", title='', button='OK')
        sensitivity = int(pymsgbox.prompt(text="Enter the sensitivity (grams) and do NOT touch the buttons", title='' , default='1'))

        cs = scipy.interpolate.CubicSpline(x_grams, y_measured, bc_type='natural')
        
        sensitivity = int(cs(sensitivity+0.3))

        send_sens_values(sensitivity, sensitivity)
        pymsgbox.alert(text="Success", title='', button='OK')



    
    

    
dpg.create_context()
dpg.create_viewport(title='Zamonarypad', width=200, height=400)
dpg.setup_dearpygui()

with dpg.window(tag="mainwin"):

    # droplist of all serial devices
    print("created window instance")
    serial_ports = get_serial_ports()
    print(serial_ports)
    serial_devices_list = dpg.add_combo(serial_ports, default_value=serial_ports[0] if serial_ports else "", tag="serial_port")

    dpg.add_button(callback=connect_serial, label='connect')
    dpg.add_button(callback=refresh_serial, label='refresh')

    button_bar_1 = dpg.add_progress_bar(label="1", default_value=0, tag="progress_bar_1")
    button_bar_2 = dpg.add_progress_bar(label="2", default_value=0, tag="progress_bar_2")
    
    dpg.add_button(callback=calibrate_btn, label='calibrate')

    dpg.add_text("", tag="connection_status")
    dpg.add_text("", tag="values")

dpg.show_viewport()
dpg.set_primary_window("mainwin", True)




while dpg.is_dearpygui_running():
    
    readbuttonvalues()
    
    
    
    dpg.set_value("progress_bar_1", json_response['button1val']/100.0)
    dpg.set_value("progress_bar_2", json_response['button2val']/100.0)
    dpg.set_value("values", (json_response['button1val_raw'], json_response['button2val_raw']))
    dpg.render_dearpygui_frame()
dpg.destroy_context()


