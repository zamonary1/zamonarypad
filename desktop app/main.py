import dearpygui.dearpygui as dpg
import pymsgbox
import serial
import serial.tools.list_ports
import time
import json

# Глобальные переменные
ser = None
lasttime = time.time()
json_response = {"button1val":0,"button2val":0,"button1sens":0,"button2sens":0}
json_response_raw = json_response
min1 = float('inf')
min2 = float('inf')
max1 = float('-inf')
max2 = float('-inf')
delay_between_reads = 0.033 #33ms between updates or 30 updates per second
# Функция для получения списка доступных последовательных портов
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

# Функция для установки последовательного соединения
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

# Функция для отправки команды "readbtn1"
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

def calibrate_btn():
    if ser is not None:
        
        #pymsgbox.alert(text="Do not touch the zamonaryboard and press OK", title='', button='OK')
        sensitivity = int(pymsgbox.prompt(text="Enter the sensitivity and do NOT touch the buttons", title='' , default='300'))
        json_response_raw = readbuttonvalues(True)
        val_btn1_idle = int(json_response_raw["button1val"])
        val_btn2_idle = int(json_response_raw["button2val"])
        print(json_response_raw, val_btn1_idle, val_btn2_idle)
        time.sleep(0.1)
        ser.write(f'wrbtn1 {val_btn1_idle + sensitivity}\n'.encode())
        time.sleep(0.1)
        ser.write(f'wrbtn2 {val_btn2_idle + sensitivity}\n'.encode())
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
    # if serial_ports == ["Not found"]:
    #     dpg.hide_item(serial_devices_list)
    # else: 
    #     dpg.show_item(serial_devices_list)

    # Кнопка для установки последовательного соединения
    dpg.add_button(callback=connect_serial, label='connect')
    dpg.add_button(callback=refresh_serial, label='refresh')

    # Кнопки для отправки команд "readbtn1" и "readbtn2"
    #dpg.add_button(callback=send_readbtn1, label='read 1')
    #dpg.add_button(callback=send_readbtn2, label='read 2')

    # Полоса загрузки для отображения полученных значений
    button_bar_1 = dpg.add_progress_bar(label="1", default_value=0, tag="progress_bar_1")
    button_bar_2 = dpg.add_progress_bar(label="2", default_value=0, tag="progress_bar_2")
    
    dpg.add_button(callback=calibrate_btn, label='calibrate')

    # Метка для отображения статуса соединения
    dpg.add_text("", tag="connection_status")

dpg.show_viewport()
dpg.set_primary_window("mainwin", True)

while dpg.is_dearpygui_running():
    
    readbuttonvalues()
    
    
    
    dpg.set_value("progress_bar_1", json_response['button1val']/100.0)
    dpg.set_value("progress_bar_2", json_response['button2val']/100.0)
    dpg.render_dearpygui_frame()
dpg.destroy_context()


