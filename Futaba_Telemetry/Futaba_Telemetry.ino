#include "SBUS2.h"
#include "SBUS_usart.h"

#define TEMPRATURE_SLOT   1     // 1 Slot Sensor
#define RPM_SLOT          2     // 1 Slot Sensor
#define CURRENT_SLOT      3     // 3 Slot Sensor
      //CURRENT_SLOT      4
      //CURRENT_SLOT      5
#define ERROR_SLOT        6     // 1 Slot Sensor

#define GPS_SLOT          8     // 8 Slot Sensor

#define VOLTAGE_SLOT      16    // 2 Slot Sensor
#define VARIO_SLOT        18    // 2 Slot Sensor


// Koordinaten Berlin Fernsehturm  = Grad Minuten  = Dezimalgrad = Grad Minuten Sekunden
int32_t latitude = 52312499;    // = N 52° 31.2499 = N 52.520833 = N 52° 31' 14.9988"
int32_t longitude = 13245658;   // = E 13° 24.5658 = E 13.409430 = E 13° 24' 33.9480"

int16_t channel = 0;


void setup() {
  // put your setup code here, to run once:

  pinMode(2, OUTPUT);             // set pin D2 as Output  
  digitalWrite(2, HIGH);          // set pin D2 High = Inverted UART signal
  pinMode(13, OUTPUT);            // set pin D10 (LED)  
  digitalWrite(13, LOW);          // set pin D10 (LED OFF)
  
  SBUS2_Setup();


  while(!SBUS2_Ready()){
    // Wait for valid SBUS2 Signal
  }
  
  // GPS Distance and Altitude is set to Zero with the first GPS Telemetry Data
  // GPS Distance and Altitude is calculated with changing Latitude, Longitude and Altitude
  for(uint8_t i = 0; i< 10; i++){
    send_f1675_gps(GPS_SLOT, (uint16_t)0, (int16_t)0, (int16_t) 0, (latitude+100), (longitude+100));           // Speed = 0km/h, Altitude = 0m, Vario = 0m/s
    send_f1672_vario(3, (int16_t) 0, (int16_t) 0);
    delay(50);
  }
  digitalWrite(13, HIGH);          // set pin D13 (LED ON) -> Setup finished
  
}

void loop() {
  uint16_t uart_dropped_frame = 0;
  bool transmision_dropt_frame = false;
  bool failsave = false;

  /* DO YOUR STUFF HERE */
  /* Make sure you do not do any blocking calls in the loop, e.g. delay()!!! */

  if(SBUS_Ready()){                             // SBUS Frames available -> Ready for getting Servo Data
    channel = SBUS2_get_servo_data( 5 );        // Channel = Servo Value of Channel 5
  }

  
  if(SBUS2_Ready()){                                                                                                // SBUS2 Frame available -> Ready for transmit Telemetry 
    digitalWrite(13, HIGH);                                                                                         // set pin D13 (LED ON) -> SBUS2 Frames OK                                      
    uint8_t FrameErrorRate = SBUS2_get_FER();
    
    send_temp125(TEMPRATURE_SLOT, (int16_t) 50);
    
    SBUS2_get_status(&uart_dropped_frame, &transmision_dropt_frame, &failsave);                                     // Check SBUS(2) Status
    send_alarm_as_temp125(ERROR_SLOT, ((failsave*1000) + (transmision_dropt_frame*100) + uart_dropped_frame));

    send_F1672(VARIO_SLOT , 1234, (float)23.45);                                                                    // Altitude = 1234m; Vario = 23,45 m/s; 

    send_f1675_gps(GPS_SLOT, (uint16_t)50, (int16_t)1000, (int16_t) 200, latitude, longitude);
    
    send_RPM(RPM_SLOT,(uint16_t)600);                                                                               // RPM = 600-> rounding Error +/- 3 RPM
    send_s1678_current(CURRENT_SLOT,(uint16_t)2345,(uint16_t)15000,(uint16_t)1234);                                 // Current = 23.45A, Capacity = 15000mAh, Voltage = 12.34V

    send_voltage(VOLTAGE_SLOT,(uint16_t)128, (uint16_t)255);                                                        // Voltage1 = 12.8V, Voltage2 = 25.5V
  }
  else{
    digitalWrite(13, LOW);                                                                                          // set pin D13 (LED OFF) -> No SBUS2 Frames
  }


} // End of Loop()
