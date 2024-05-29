import dearpygui.dearpygui as dpg
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
delay_between_reads = 0.03
# Функция для получения списка доступных последовательных портов
def get_serial_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

# Функция для установки последовательного соединения
def connect_serial():
    global ser
    port = dpg.get_value("serial_port")
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        dpg.set_value("connection_status", f"Connected to {port}")
    except serial.SerialException:
        ser = None
        dpg.set_value("connection_status", "Failed to connect")

def refresh_serial():
    dpg.configure_item("serial_port", items=get_serial_ports())

# Функция для отправки команды "readbtn1"
def readbuttonvalues(mode = ''):
    global lasttime
    global json_response
    global json_response_raw
    global min1
    global min2
    global max1
    global max2
    
    if lasttime + delay_between_reads < time.time() and ser != None:
        ser.write(b'read\n')
        json_response_raw = ser.readline()
        json_response_raw = json.loads(json_response_raw)
        json_response = json_response_raw
        print(json_response)
        
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
    
    if mode == 'raw': return(json_response_raw)
    return(json_response)

def calibrate_btn_1():
    if ser is not None:
        json_response_raw = readbuttonvalues('raw')
        val_to_calibrate = json_response_raw["button1val"]
        print(val_to_calibrate)
        ser.write(f'wrbtn1 {val_to_calibrate}\n'.encode())
        ser.readline()


    
    

    
dpg.create_context()
dpg.create_viewport(title='Zamonarypad', width=200, height=400)
dpg.setup_dearpygui()

with dpg.window(tag="mainwin"):

    # Выпадающий список для выбора последовательного порта
    print("created window instance")
    serial_ports = get_serial_ports()
    print(serial_ports)
    dpg.add_combo(serial_ports, default_value=serial_ports[0] if serial_ports else "", tag="serial_port")

    # Кнопка для установки последовательного соединения
    dpg.add_button(callback=connect_serial, label='connect')
    dpg.add_button(callback=refresh_serial, label='refresh')

    # Кнопки для отправки команд "readbtn1" и "readbtn2"
    #dpg.add_button(callback=send_readbtn1, label='read 1')
    #dpg.add_button(callback=send_readbtn2, label='read 2')

    # Полоса загрузки для отображения полученных значений
    dpg.add_progress_bar(label="1", default_value=0, tag="progress_bar_1")
    dpg.add_progress_bar(label="2", default_value=0, tag="progress_bar_2")
    
    dpg.add_button(callback=calibrate_btn_1, label='calibrate 1')

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


