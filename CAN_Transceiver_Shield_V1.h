/* 
CAN_Transeiver_Shield_V1 Header
April 2024
Enes M. Özcan, Maximilian Polevka

Header includes several includes, defines, function prototypes and typedefs
Some adjustements can take place here, e.g. task priorities, buffer lengths, choose network, choose UTF-8 or OBD Frame

*/


/* includes */
#include <WiFi.h>
#include <WebServer.h>
#include "CANCommunicationWebPage.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/twai.h"
#include "soc/soc.h"
#include <stdlib.h>
#include <stdio.h>

/* defines */
#define USE_INTRANET  //if defined then connecting to access point, else creating access point

#define LOCAL_SSID "your_local_ssid"        
#define LOCAL_PASS "your_local_password"

#define AP_SSID "TestWebsite"
#define AP_PASS "0123456789"

// choose Task priorities: see FreeRTOS Reference Manual Version 10.0.0 issue 1 p.35 --> higher number means higher priority
#define CAN_CONTROLLER_RECEIVE_TASK_PRIO        9         
#define CAN_CONTROLLER_TRANSMIT_TASK_PRIO       8                
#define WIFI_TASK_PRIO                          7  
#define CONTROL_TASK_PRIO                       6

// factors for multiplying raw data of ADC Output Data, float instead of double cause ESP32-S3 has got a FPU

#define FACT_ADC_5V (3.3f / 4096.0f) * ((10000.0f + 10000.0f) / 10000.0f)    // for checking (over- / under-) voltage of 5V LDO (IC2). Voltage divider see schematic (R2 = 10k, R5 = 10k)
#define FACT_ADC_3V3 (3.3f / 4096.0f) * ((5000.0f + 27000.0f) / 27000.0f)    // for checking (over- / under-) voltage of 3.3V LDO (IC1). Voltage divider see schematic (R1 = 5k, R4 = 27k)

// define Pins 
#define PIN_ADC_3V3 2
#define PIN_ADC_5V 7
#define LED_GREEN 35
#define LED_RED 36
#define LED_BLUE 37
#define LBK_PIN 21
#define CAN_TX_PIN 38
#define CAN_RX_PIN 1

#define TX_QUEUE_LEN 20                   // set length of RX and TX Queue of TWAI Controller
#define RX_QUEUE_LEN TX_QUEUE_LEN

//#define USE_UTF8_ENCODING                 // Define, if UTF-8 message should be send. Otherwise (OBD) do not define.

#ifdef USE_UTF8_ENCODING
  #define TX_BUF_LEN 50
  #define RX_BUF_LEN TX_BUF_LEN
  #define EXTENDED_ID 1
  #define ID_LEN 29
#else
  #define TX_BUF_LEN 20
  #define RX_BUF_LEN TX_BUF_LEN
  #define EXTENDED_ID 0
  #define ID_LEN 11
#endif 


/* function prototypes */
static void cancontroller_transmit_task(void *arg);
static void cancontroller_receive_task(void *arg);
static void wifi_task(void *arg);
static void control_task(void *arg);

void Eingabe_ID(void);                                  // callback function of input ID on website
void Eingabe_Botschaft(void);                           // callback function of input message on website
void CAN_send(void);                                    // put CAN Frame on Bus
void ProcessButton_0(void);                             // callback function of reset button on website
void SendWebsite(void);                                   
void SendXML(void);
void printWifiStatus(void); 

/* typedef */

typedef enum                                            // typedef enum for choosing messagetype 
{
  UTF8_message,
  OBD_message
} messagetype_t;

typedef enum                                            // typedef enum for choosing Bitrate
{
  timing_250K,
  timing_500K,
  timing_800K,
  timing_1M
} settwaitiming_t;


