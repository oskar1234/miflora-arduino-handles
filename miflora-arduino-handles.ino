#include <SoftwareSerial.h>

// -----------------------------------------------------------------------
// Author:  Patrick Schultes
// Year:    2022
// License: MIT
// ------
// The "Magic Value" and the parsing logic is documented on this page: 
// https://github.com/ChrisScheffler/miflora/wiki/The-Basics
// So Special thanks @Chris
//
// Tested with the HM-10 Firmware v710 and MiFlora Firmware 3.3.5.
//
// Very good Docu to the HM-10 and BLE: http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/
// Some parts are needed before you can start. 
// 0. Usually a firmware Update. You NEED the "Self-Learning Functions" --> So V700 or higher
// 1. Set your HM-10 in "Master Mode": AT+ROLE1
// 2. Set your HM-10 in "Manual Connect": AT+IMME1
// 3. Set the Baudrate to 19200: AT+BAUD1
// -----------------------------------------------------------------------

// Comment this line if you do not want to have the debug info
#define DEBUG

// -----------------------------------------------------------------------
// Defines and global Variables

#define BT_ADRESS           "C47C8D6DD7D1"
#define CMD_HM10_VERSION    "AT+VERR?"
#define CMD_CONNECT         "AT+CON"        // CMD_CONNECT + BT_ADRESS
#define CMD_SET_MODE        "AT+SET_WAYWR"  // CMD_SET_MODE + HANDLE
#define CMD_READ            "AT+READDATA"   // CMD_READ + HANDLE + "?"
#define HANDLE_SENSOR_DATA  "0035"
#define HANDLE_MODE         "0033"
#define HANDLE_BAT_VER      "0038"

#define RETURN_CONNECT_REFUSED  "OK+CONNF"
#define RETURN_CONNECT_SUCESS   "OK+CONN"
#define RETURN_CONNECT_RETURN   "OK+CONNA"
#define RETURN_SEND_RETURN      "OK+SEND-OK"
#define LENGTH_USABLE_DATA      16

// Magic value...
uint8_t g_magic_ua8[] = {0xA0, 0x1F};

void parse_data(uint8_t f_data_stream_ua8[]);
SoftwareSerial g_serialbluetooth_h(10, 11); // RX, TX (Arduino side)

// -----------------------------------------------------------------------
// Start Setup
void setup() {
  g_serialbluetooth_h.begin(19200);
  g_serialbluetooth_h.flush();
  Serial.begin(19200);

  delay(2000);
  Serial.println("Start Serial");
  delay(1000);

  // Sometimes this part is not working probably
  // It seams that a buffer is still filled
  g_serialbluetooth_h.write(CMD_HM10_VERSION);
  while (!g_serialbluetooth_h.available()) {} // Wait for the Response
  String l_temp_str = g_serialbluetooth_h.readString();
  Serial.print("HM-10 Version: ");
  Serial.println(l_temp_str);
  delay(1000);
}

void loop() {
  // Connect to MiFlora
  g_serialbluetooth_h.write("AT+CONC47C8D6DD7D1");

  // Wait till response is complete
  // Unfortanetly the response take a while and off course not "Real Time"...
  delay(2000); 

  // Get response if commat is set
  String l_temp_str = g_serialbluetooth_h.readString();
  
  #ifdef DEBUG
  Serial.println(l_temp_str);
  #endif

  // Delete the normal command return to check if the connection was succesful
  l_temp_str.replace(RETURN_CONNECT_RETURN, "#");
  Serial.println(l_temp_str);
  if (l_temp_str.indexOf(RETURN_CONNECT_SUCESS) != -1)
  {
    #ifdef DEBUG
    Serial.println("Connection Successful");
    #endif

    // Set the mode to "writing" for handly 0033
    g_serialbluetooth_h.write("AT+SET_WAYWR0033"); 
    delay(100);
    // Send the magic values as raw integer (not ASCII!)
    g_serialbluetooth_h.write(g_magic_ua8,2);
    delay(100);
    // Readreqeust for the Sensor Data
    g_serialbluetooth_h.write("AT+READDATA0035?");
    delay(500);

    // Read the Data
    String l_temp_2_str = g_serialbluetooth_h.readString();

    // Use a local byte buffer for the Data
    uint8_t l_usable_data_ua8[LENGTH_USABLE_DATA];
    // Unfortanetly sometimes there is an "0" at the end and sometimes not
    // So I search for the response and from this the Data is located 5 index later... if there is no "0"
    uint8_t l_start_value_u8 = l_temp_2_str.indexOf("-OK") + 5;

    if (0x9B != (uint8_t)l_temp_2_str[l_start_value_u8+ LENGTH_USABLE_DATA - 1])
    {
      // Sometime a strange "0" is in the stream...
      // And the 0x9B is everytime at the end
      l_start_value_u8++;
    }
    
    // Copy the data to the buffer
    for (uint8_t i = l_start_value_u8 ; i < l_start_value_u8 + LENGTH_USABLE_DATA; i++)
    {
        // Convert the Stream to a uint8 Array
        l_usable_data_ua8[i -l_start_value_u8] = (uint8_t)l_temp_2_str[i];
        #ifdef DEBUG
        Serial.print((uint8_t)l_temp_2_str[i],HEX);
        Serial.print("-");
        #endif
    }
    #ifdef DEBUG
    Serial.println("\n"+l_temp_2_str);
    #endif
    parse_data(l_usable_data_ua8);
  }
  else
  {
    Serial.println("Connection Refused");
  }
  #ifdef DEBUG  
  Serial.println("Alive");
  #endif
  delay(15000);
  g_serialbluetooth_h.readString();
  g_serialbluetooth_h.flush();
}

void parse_data(uint8_t f_data_stream_ua8[])
{
  // See the website from chris if you need the info about parsing

  uint32_t l_temp_u32 = 0; 
  l_temp_u32 = f_data_stream_ua8[1] << 8;
  l_temp_u32 = l_temp_u32 | f_data_stream_ua8[0];
  float temperature = float(l_temp_u32) / 10;
  
  l_temp_u32 = 0; 
  l_temp_u32 = f_data_stream_ua8[6] << 24;
  l_temp_u32 = l_temp_u32 | f_data_stream_ua8[5] << 16;
  l_temp_u32 = l_temp_u32 | f_data_stream_ua8[4] << 8;
  l_temp_u32 = l_temp_u32 | f_data_stream_ua8[3];
  uint32_t l_lux_u32 = l_temp_u32; 

  uint8_t l_moisture_u8 = f_data_stream_ua8[7];

  l_temp_u32 = 0; 
  l_temp_u32 = f_data_stream_ua8[9] << 8;
  l_temp_u32 = l_temp_u32 | f_data_stream_ua8[8];
  uint16_t l_conductivity_u16 = (uint16_t)l_temp_u32;
  
  #ifdef DEBUG  
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Lux: ");
  Serial.println(l_lux_u32);
  Serial.print("Moisture: ");
  Serial.println(l_moisture_u8);
  Serial.print("Conductivity: ");
  Serial.println(l_conductivity_u16);
   #endif
}
