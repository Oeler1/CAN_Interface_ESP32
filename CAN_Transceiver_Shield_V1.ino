/* 
CAN_Transeiver_Shield_V1 Program
April 2024
Enes M. Özcan, Maximilian Polevka

This programme can be used to enter and receive CAN messages via a website. The CAN communication takes place via an ESP32 shield with CAN transceiver, which communicates with the TWAI controller. 
The programme parts are the main programme "CAN_Transceiver_Shield_V1.ino", the header "CAN_Transceiver_Shield_V1.h" with function prototypes and defines as well as the header "CANCommunicationWebPage.h", which contains HTML and JavaScript.
Either text messages with up to 49 bytes or OBD messages can be sent (must be selected beforehand via defines).
To send messages with UTF-8 encoding, "USE_UTF8_ENCODING" must be defined in the header file. 
(The number of characters is set to 49, as a line feed is added to the end of a message to detect this. The length of the buffer can be adjusted with the defines "RX_BUF_LEN" and "TX_BUF_LEN".)
Otherwise, messages can only be sent and received as OBD frames (8 bytes of data as a HEX number).
Before start-up, the SSID and the corresponding network pass must be entered so that the microcontroller establishes a connection to the desired network.

*/

/* includes */

#include "CAN_Transceiver_Shield_V1.h"

/* defines */
#ifdef USE_UTF8_ENCODING                               
  const messagetype_t CAN_MessageType = UTF8_message;             // constant so value does not change
#else
  const messagetype_t CAN_MessageType = OBD_message;
#endif

settwaitiming_t myTWAItiming = timing_800K;                     // set Bitrate of CAN Bus choose between 250 kBit/s, 500 kBit/s, 800 kBit/s, 1 MBit/s. Otherwise timing registers (prescaler, etc.) need to be written manually
uint32_t alerts_to_enable = TWAI_ALERT_RX_QUEUE_FULL | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF;                    // choose which alerts are about to get enabled 
// uint32_t alerts_to_enable = TWAI_ALERT_ALL;                  // enable all alerts

/* variables */

uint32_t ADC_3V3_RAW = 0, ADC_5V_RAW = 0;                // variables for digital Data Value of converted voltage (RAW)
float VoltsADC_3V3 = 0.0f, VoltsADC_5V = 0.0f;                // converted values (multiplicated with factor to get value in Volts)
bool BUTTON0 = false;                             // Reset Button Value
uint32_t SensorUpdate = 0;                        // for ADC timing

/* Wifi variables */

char XML[2500];
char buf[60];                                     

IPAddress Actual_IP;
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

WebServer server(80);

// several Buffers for CAN <--> Wifi Interaction
uint32_t ID_transmit = 0;                                       
uint64_t Botschaft_transmit = 0;
char ID_receive[ID_LEN + 1] = {'\0'};              // +1 to add null terminator to string       
char Botschaft_receive[RX_BUF_LEN] = {'\0'}; 

/* CAN variables */

//buffers
uint8_t tx_buf[TX_BUF_LEN];
uint8_t rx_buf[RX_BUF_LEN];
char control_message_buf [250];
 
twai_message_t rx_message;                            // CAN Receive message
twai_message_t tx_message;                            // CAN Transmit message
twai_status_info_t mystatus;                          // TWAI Controller status
twai_timing_config_t t_config;                        // TWAI Timing Options


uint8_t *receive_ptr;                                 // pointer to load received message into buffer
uint16_t rx_counter = 0;                              // number of received bytes via CAN
uint16_t tx_string_len = 0;                           // length of String to be transmitted via CAN

bool readytosend_id = false;                          // ID received from Website ready to be put on CAN Bus
bool readytosend_message = false;                     // message received from Website ready to be put on CAN Bus
bool end_of_message_flag = false;                     // if true: data received from CAN Bus is ready to be processed
int receive_data_ready_flag = 0;                      // receive CAN message is ready to be put on Website
uint32_t alerts_triggered;

/* semaphores */
// semaphores are used to eliminate conflicts in data processing that can otherwise arise when different tasks access a shared memory (like global variables)

static SemaphoreHandle_t can_send_id_write_sem; 
static SemaphoreHandle_t can_send_id_read_sem;
static SemaphoreHandle_t can_send_message_write_sem;
static SemaphoreHandle_t can_send_message_read_sem;

static SemaphoreHandle_t can_receive_write_sem;
static SemaphoreHandle_t can_receive_read_sem;

static SemaphoreHandle_t wifi_setup_complete;        // semaphore to not interrupt wifi setup by other tasks

void setup() 
{
  Serial.begin(115200);               // configure an start Serial Port for Information Output with 115200 Bit/s
  
  pinMode(LED_GREEN, OUTPUT);         // configure green LED
  digitalWrite (LED_GREEN, LOW);

  pinMode(LED_RED, OUTPUT);           // configure red LED
  digitalWrite (LED_RED, LOW); 

  pinMode(LED_BLUE, OUTPUT);          // configure blue LED
  digitalWrite (LED_BLUE, LOW);  

  /* set Loopback Mode of Transceiver connected to Pin 21: Low --> Loopback Mode off, High --> Loopback Mode active */
  pinMode (LBK_PIN, OUTPUT);           
  digitalWrite (LBK_PIN, LOW); 

  // reset Buffer and wait a bit cause memory operations can be a bit slow 
  memset(tx_buf, '\0', TX_BUF_LEN);    
  memset(rx_buf, '\0', RX_BUF_LEN);
  delay(100);  

  /* create new binary semaphore instances */
  can_send_id_write_sem = xSemaphoreCreateBinary();                         // semamphores are default set taken
  can_send_id_read_sem = xSemaphoreCreateBinary();
  can_send_message_write_sem = xSemaphoreCreateBinary();
  can_send_message_read_sem = xSemaphoreCreateBinary();
  
  can_receive_write_sem = xSemaphoreCreateBinary();
  can_receive_read_sem = xSemaphoreCreateBinary();

  wifi_setup_complete = xSemaphoreCreateBinary();

  //Initialize TWAI configuration structures

  /* twai configurations */
  twai_general_config_t g_config =  { .mode = TWAI_MODE_NORMAL,                  // set TWAI MODE to one of the following: TWAI_MODE_NORMAL, TWAI_MODE_NO_ACK (for testing), TWAI_MODE_LISTEN_ONLY
                                      .tx_io = (gpio_num_t) CAN_TX_PIN,          // set TX Pin of Transceiver
                                      .rx_io = (gpio_num_t) CAN_RX_PIN,          // set TX Pin of Transceiver
                                      .clkout_io = TWAI_IO_UNUSED,               // set optional output pin of  prescaled version of the controller's source clock (APB Clock)
                                      .bus_off_io = TWAI_IO_UNUSED,              // set optional pin to get Low when TWAI controller reaches BUS-OFF state, gets High otherwise
                                      .tx_queue_len = TX_QUEUE_LEN,              // Number of messages TX queue can hold (set to 0 to disable TX Queue)
                                      .rx_queue_len = RX_QUEUE_LEN,              // Number of messages RX queue can hold
                                      .alerts_enabled = TWAI_ALERT_NONE,         // Bit field of alerts to enable
                                      .clkout_divider = 0,                       // Can be 1 or any even number from 2 to 14 (optional, set to 0 if unused)
                                      .intr_flags = ESP_INTR_FLAG_LEVEL1         // set Interrupt and priority (see documentation)
                                    };                                           
  
  /* timing config */

  switch (myTWAItiming)
  {
    case timing_250K:
      t_config = TWAI_TIMING_CONFIG_250KBITS();
      break;

    case timing_500K:
      t_config = TWAI_TIMING_CONFIG_500KBITS();
      break;
    
    case timing_800K:
      t_config = TWAI_TIMING_CONFIG_800KBITS();
      break;

    case timing_1M:
      t_config = TWAI_TIMING_CONFIG_1MBITS();
      break;
    
   default:
      Serial.println("Timing option not defined.");
      break;
  } 

  /* filter config */
  // if single filter is desired, configurate manually like following:
  // example for using single filter for extended ID: twai_filter_config_t f_config = {.acceptance_code = (uint32_t) (0x0000AAAA << 3), .acceptance_mask = (uint32_t) (0x00000003 << 3), .single_filter = true};
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  //Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) 
  {
    Serial.println("Driver installed");
  } 
  else 
  {
    Serial.println("Failed to install driver");
    return;
  }

  //Start TWAI driver
  if (twai_start() == ESP_OK) 
  {
    Serial.println("Driver started");
    digitalWrite(LED_BLUE, HIGH);
  } 
  else 
  {
    Serial.println("Failed to start driver");
    return;
  }

  // Reconfigure alerts to detect all alerts
  // if only individual alerts should be active do as follows:
  // uint32_t alerts_to_enable = TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_OFF;

  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) 
  {
    printf("Alerts reconfigured\n");
  } 
  else 
  {
    printf("Failed to reconfigure alerts");
  }
 

  receive_ptr = rx_buf;                 // set receive pointer to first element of TWAI RX Buffer
  tx_message.rtr = 0;                   // tx message is not a remote frame 
  tx_message.extd = EXTENDED_ID;        // set ID length (either 29 or 11 Bit, value gets set with defines UTF8: 29 Bit (EXTEDNDED ID = 1), OBD: 11 Bit (EXTENDED_ID = 0))

  // create Tasks tskNO_AFFINITY --> not pinned to any CPU, scheduler decides core
  xTaskCreatePinnedToCore(wifi_task, "WIFI_task", 4096, NULL, WIFI_TASK_PRIO, NULL, 0);           // Wifi Tasks need to be running on Core 0
  xSemaphoreTake(wifi_setup_complete, portMAX_DELAY);                                             // is given at the end of wifi setup, take to resume program
  
  xTaskCreatePinnedToCore(cancontroller_transmit_task, "CANCONTROLLER_TRANSMIT_task", 4096, NULL, CAN_CONTROLLER_TRANSMIT_TASK_PRIO, NULL, 1);    // Other tasks are allowed to run on Core 1 --> choose Core 1 to not block Kernel Protocol Task on Core 0 
  xTaskCreatePinnedToCore(cancontroller_receive_task, "CANCONTROLLER_RECEIVE_task", 4096, NULL, CAN_CONTROLLER_RECEIVE_TASK_PRIO, NULL, 1);
  xTaskCreatePinnedToCore(control_task, "CONTROL_task", 4096, NULL, CONTROL_TASK_PRIO, NULL, 1);

  // give write semaphores, otherwise program cannot resume
  xSemaphoreGive(can_send_id_write_sem);                     
  xSemaphoreGive(can_send_message_write_sem);
  xSemaphoreGive(can_receive_write_sem);

  vQueueDelete(wifi_setup_complete);      // wifi_setup_complete semaphore is not used anymore --> clear memory

}

void loop()                 // loop task runs also on core 1
{
  // not used
}  

static void cancontroller_transmit_task(void *arg)
{
  
  while(1)
  {
    switch(CAN_MessageType)                                                 // choose between UTF-8 oder OBD Message
    {
      case UTF8_message:
        xSemaphoreTake(can_send_id_read_sem, portMAX_DELAY);                // take semaphores if set given by website input callback
        xSemaphoreTake(can_send_message_read_sem, portMAX_DELAY);

        if(readytosend_id && readytosend_message)                           // check if ID and message from Website can be read
        {
          if(tx_string_len < 8)                                             // if message fits in one frame
          {  
            tx_message.data_length_code = (tx_string_len + 1);

            for (int i = 0; i < (tx_string_len + 1); i++) 
            {
              tx_message.data[i] = tx_buf[i];
              Serial.print("tx_buf["); 
              Serial.print(i);
              Serial.print("] = ");
              Serial.println((char) tx_buf[i]);

            }          
            CAN_send();
          }
          else if(tx_string_len < TX_BUF_LEN)                               // if message needs to be put in several frames
          {
            uint8_t send_iters = (tx_string_len / 8U);                      // compute quantity of full data frames (8 Byte) in need
            uint8_t send_rest = (tx_string_len % 8U);                       // compute size of last data frame
            uint8_t *send_ptr = tx_buf;
  	        tx_message.data_length_code = 8;

            for(int i = 0; i < send_iters; i++)                             // fill full data frames 
            {
              for(int j = 0; j < 8; j++)
              {
                tx_message.data[j] = *send_ptr;      
                send_ptr++;           
              }
              CAN_send();
              vTaskDelay(pdMS_TO_TICKS(10));
            }
    
            tx_message.data_length_code = (send_rest+1);                    // fill last data frame

            for(int i = 0; i < (send_rest+1); i++)
            {
              tx_message.data[i] = *send_ptr;      
              send_ptr++;        
            }
            CAN_send();
          }
          else                                                            // message too large, eiter adjust buffer or send shorter messages
          {
            Serial.print("String is out of accepted length (");
            Serial.print(TX_BUF_LEN);
            Serial.println(").");
          }
        }
        xSemaphoreGive(can_send_id_write_sem);                            // give write access back
        xSemaphoreGive(can_send_message_write_sem);
        break;

      case OBD_message:

        xSemaphoreTake(can_send_id_read_sem, portMAX_DELAY);              // take semaphores if set given by website input callback
        xSemaphoreTake(can_send_message_read_sem, portMAX_DELAY);       

        if(readytosend_id && readytosend_message)                         // fill data frame
        {  
          tx_message.data_length_code = 8;
          
          for (int i = 0; i < 8; i++) 
          {
            tx_message.data[i] = tx_buf[i];
            Serial.print("tx_buf["); 
            Serial.print(i);
            Serial.print("] = ");
            Serial.println( tx_buf[i], HEX);

          }          
          CAN_send();     
        }       

        xSemaphoreGive(can_send_id_write_sem);
        xSemaphoreGive(can_send_message_write_sem);
        break;

      default:
        Serial.println("Fault: cancontroller_transmit_task in default state");
        break;       
    } 
  }

  vTaskDelete(NULL);
}

static void cancontroller_receive_task(void *arg)
{ 

  while(1)
  {
    twai_get_status_info(&mystatus);                                  // check twai status (gives back RX Queue Length)

    switch(CAN_MessageType)                                           // choose between UTF-8 oder OBD Message
    {      
      case UTF8_message:    
        if(mystatus.msgs_to_rx > 0)                                     // if RX Queue is not empty request rx messages
        {   
          if (twai_receive(&rx_message, pdMS_TO_TICKS(10)) == ESP_OK)    // rx message request
          {
            Serial.println("CAN Message received");
          }

          if (!(rx_message.rtr))                                        // check if rx message is not a remote frame 
          {
            for (int i = 0; i < rx_message.data_length_code; i++)       // if not put message into rx buffer
            {
              rx_counter++;

              if((rx_message.data[i]) == 10)                            // set end_of_message flag if Linefeed is detected ('\n' --> 10)
              {         
                end_of_message_flag = true;
              }

              *receive_ptr = (rx_message.data[i]);
              receive_ptr++;
            }
          }

          if(end_of_message_flag)                                     // put received message into buffer to put it on website
    	    {
            xSemaphoreTake(can_receive_write_sem, portMAX_DELAY);
            receive_ptr = rx_buf;
            end_of_message_flag = false;
        
            Serial.print("Received String: ");        
            for(int i = 0; i < (rx_counter-1); i++)
            {
              Serial.print((char) (rx_buf[i]));
              Botschaft_receive[i] = (char) rx_buf[i];
            }
            Botschaft_receive[rx_counter-1] = '\0';
            Serial.println("."); 

            sprintf((char *) ID_receive, "%X\0", rx_message.identifier);         

            rx_counter = 0;                                            // set back rx counter for new message
            receive_data_ready_flag = 1;                               // give information to other task, that buffers are filled and can be put on website
          }  
        }
        xSemaphoreGive(can_receive_read_sem);
        vTaskDelay(pdMS_TO_TICKS(50));                                 // Delay only for this task, not whole core
        break;
      
      case OBD_message:
        if(mystatus.msgs_to_rx > 0)                                 
        {
          memset(Botschaft_receive, '\0', RX_BUF_LEN);                      // reset RX_BUF otherwise strcat for filling buffer does not work porperly

          if (twai_receive(&rx_message, pdMS_TO_TICKS(10)) == ESP_OK) 
          {
            if (!(rx_message.rtr))                                          // check if rx message is not a remote frame
            {
              receive_ptr = rx_buf;

              for (int i = 0; i < rx_message.data_length_code; i++)         // put message into rx buffer
              {
                *receive_ptr = (rx_message.data[i]);
                receive_ptr++;
              }              
              
              xSemaphoreTake(can_receive_write_sem, portMAX_DELAY);                                       
        
              Serial.print("Received Hex: ");                    
              for(int i = 0; i < 8; i++)                                    // convert Hex integer into String with OBD format and put into message buffer
              {
                Serial.print((rx_buf[i]), HEX);                           
                char temp_buf[2] = {'\0'};
                
                sprintf(temp_buf, "%02X", rx_buf[i]);
                strcat(Botschaft_receive, temp_buf);                              
              }
              Botschaft_receive[16] = '\0';                                // add null terminator to message                         
              Serial.println("."); 

              for(int j = 0; j < 17; j++)
              {
                Serial.print("Botschaft_receive: ");
                Serial.println(Botschaft_receive[j]);
              }             

              sprintf((char *) ID_receive, "%X\0", rx_message.identifier);  // convert Integer ID into String, add null terminator and put into ID buffer       
            }
          } 
          receive_data_ready_flag = 1;                                      // message and ID are allowed to be sent to website
        }
        xSemaphoreGive(can_receive_read_sem);                               // give semaphore to read access 
        vTaskDelay(pdMS_TO_TICKS(10));  
        break;
      
      default:
        Serial.println("Fault: cancontroller_receiver_task in default state");
        break;
    }

  }

  vTaskDelete(NULL);
}

static void wifi_task(void *arg)
{
  // We start by connecting to a WiFi network

  #ifdef USE_INTRANET                       //connecting to existing access point
    WiFi.begin(LOCAL_SSID, LOCAL_PASS);
    while (WiFi.status() != WL_CONNECTED) 
    {
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.print(".");
    }
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    Actual_IP = WiFi.localIP();  
  #endif

  #ifndef USE_INTRANET                    //creating own access point
    WiFi.softAP(AP_SSID, AP_PASS);
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.softAPConfig(PageIP, gateway, subnet);
    vTaskDelay(pdMS_TO_TICKS(100));
    Actual_IP = WiFi.softAPIP();
    Serial.print("IP address: "); Serial.println(Actual_IP);
  #endif
   
  printWifiStatus();
  
  //these calls handle data coming back from the web page

  server.on("/", SendWebsite);  //page request, upon getting / string the web page will be sent
     
  //when getting /xml string, ESP will build and send the XML, refreshes some information on the web page
  server.on("/xml", SendXML);

  //executes the functions upon getting the strings
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/EINGABE_ID", Eingabe_ID);
  server.on("/EINGABE_Botschaft", Eingabe_Botschaft);

  server.begin(); //begins the server

  Serial.println("Server started");

  xSemaphoreGive(wifi_setup_complete);

  while(1)
  {
    server.handleClient(); //needs to be sent frequently for the web page to get instructions
  }

  vTaskDelete(NULL);
}

static void control_task(void *arg)                                // task for 
{

  while(1)
  {
    if ((millis() - SensorUpdate) >= 50)                            // read adc values if 50 ms are passed and convert into volts
    {                                                               // can be used to check over / undervoltage in further version
      SensorUpdate = millis();
      ADC_3V3_RAW = analogRead(PIN_ADC_3V3);
      ADC_5V_RAW = analogRead(PIN_ADC_5V);   

      VoltsADC_3V3 = ((float) ADC_3V3_RAW) * FACT_ADC_3V3;                             
      VoltsADC_5V = ((float) ADC_5V_RAW) * FACT_ADC_5V;  
    }

    twai_get_status_info(&mystatus);

    if(mystatus.state == TWAI_STATE_BUS_OFF)                        // check if TWAI is in Bus off State
    {
      digitalWrite(LED_RED, HIGH);
      Serial.print("TWAI in Bus-off state."); 
    }

    twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(100));            // check for alerts

    if(alerts_triggered > 0)
    {   
      sprintf(control_message_buf, "These alerts are set:\n");

      if((alerts_triggered) & (1 << TWAI_ALERT_RX_QUEUE_FULL))          // check for RX QUEUE FULL alert
      {        
        char temp_buf[] = "RX Queue is full.\n";                
        strcat(control_message_buf, temp_buf);        
      }
      if((alerts_triggered) & (1 << TWAI_ALERT_ERR_PASS))                // check if Controller has become error passive
      {
        digitalWrite(LED_RED, HIGH);
        char temp_buf[] = "TWAI controller has become error passive.\n";        
        strcat(control_message_buf, temp_buf);
      }      
      if((alerts_triggered) & (1 << TWAI_ALERT_BUS_OFF))                  // check if Bus-off condition occured
      {
        digitalWrite(LED_RED, HIGH);
        char temp_buf[] = "Bus-off condition occurred.\n";        
        strcat(control_message_buf, temp_buf);
      }
      Serial.println(control_message_buf);                                // if more alerts should be checked, more if statements have to be implemented (perhaps buffer needs to be extended)

    }


    
    vTaskDelay(pdMS_TO_TICKS(500));                                 // Task does not need to be checked that much

  }

  vTaskDelete(NULL);
}

void Eingabe_ID(void)                                                 // ID Callback
{  
  String ID_s = server.arg("IDENTIFICATION");                         // get ID from Website

  xSemaphoreTake(can_send_id_write_sem, portMAX_DELAY); 

  char * succ;                                        
  ID_transmit = strtoul(ID_s.c_str(), &succ, 16);                     // convert String ID into HEX         

  tx_message.identifier = ID_transmit;                                // set ID for TWAI controller

  Serial.println("Eingabe_ID"); 
  Serial.println(ID_s);
  Serial.println(ID_transmit);

  readytosend_id = true;                                              // ID set, ready to transmit can message
  xSemaphoreGive(can_send_id_read_sem);

  server.send(200, "text/plain", buf);                                //sends back to web page to keep the web page live
}
 
void Eingabe_Botschaft(void)                                          // message Callback
{  
  String Botschaft_s = server.arg("VALUE");                           // get message from Website
  xSemaphoreTake(can_send_message_write_sem, portMAX_DELAY);  

  tx_string_len = strlen(Botschaft_s.c_str());                        // get String length of message

  if(CAN_MessageType == UTF8_message)                                 // if UTF-8, put String message into tx_buffer and add linefeed
  {
    for(int i = 0; i < (tx_string_len); i++)
    {
      tx_buf[i] = Botschaft_s[i];
    }
    tx_buf[tx_string_len] = '\n';
  }
  
  char* temp_ptr;                                    
  Botschaft_transmit = strtoull(Botschaft_s.c_str(), &temp_ptr, 16);     // convert String to 64 Bit unsigned Integer as HEX  
  
  if(CAN_MessageType == OBD_message)                                     // put message into tx_buf (8 Bit unsigned Integer) --> every 
  {
    for(int i = 0; i < 8; i++)
    {      
      tx_buf[i] = (Botschaft_transmit >> ((7 - i) * 8)) & 0xFF;          // break down 64 Bit HEX message in 8 Bit HEX pieces to be able to send over CAN
    } 
  } 


  Serial.println("Eingabe_Botschaft"); 
  Serial.println(Botschaft_s);
  
  readytosend_message = true;                                              //flag for transmitting
  xSemaphoreGive(can_send_message_read_sem); 
  server.send(200, "text/plain", buf);                                     //feedback for the web page
} 

void CAN_send(void)                                                         // put message on CAN Bus
{
  //Queue message for transmission
  if (twai_transmit(&tx_message, pdMS_TO_TICKS(100)) == ESP_OK) 
  {
    Serial.println("CAN Message queued for transmission");    
  } 
  else 
  {
    Serial.println("Failed to queue CAN message for transmission");
  } 

  readytosend_id = false;                                                  //after transmitting->flags=false
  readytosend_message = false;
}

void ProcessButton_0(void)                                                 // Callback function Reset Button: stop and restart TWAI driver
{
 BUTTON0= !BUTTON0;
  digitalWrite(LED_GREEN, HIGH);
  twai_stop();
  digitalWrite(LED_BLUE, LOW);
  vTaskDelay(2000);

  //Start TWAI driver
  if (twai_start() == ESP_OK) 
  {
    Serial.println("Driver started");
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
  } 
  else 
  {
    Serial.println("Failed to start driver");
    return;
  }


  Serial.print("Button 0 "); Serial.println(BUTTON0);
  
  server.send(200, "text/plain", "");
}

void SendWebsite(void) 
{
  Serial.println("sending web page");
  //Sendet die PAGE_MAIN an die Webseite. PAGE_MAIN ist der header (CANCommunicationWebPage.h) mit dem kompletten html/javascript code
  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the main web page
void SendXML(void) 
{
  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // send bitsA0
  sprintf(buf, "<B0>%d</B0>\n", ADC_3V3_RAW); //BitsADC3V3 Ausgabe
  strcat(XML, buf);
  // send Volts0
  sprintf(buf, "<V0>%d.%d</V0>\n", (int) (VoltsADC_3V3), abs((int) (VoltsADC_3V3 * 10)  - ((int) (VoltsADC_3V3) * 10))); //VoltsADC3V3 Ausgabe
  strcat(XML, buf);

  // send bits1
  sprintf(buf, "<B1>%d</B1>\n", ADC_5V_RAW);  //BitsADC5V Ausgabe
  strcat(XML, buf);
  // send Volts1
  sprintf(buf, "<V1>%d.%d</V1>\n", (int) (VoltsADC_5V), abs((int) (VoltsADC_5V * 10)  - ((int) (VoltsADC_5V) * 10)));   //VoltsADC5V Ausgabe
  strcat(XML, buf);

  // send CAN-ID an XML
  sprintf(buf, "<ID0>%d</ID0>\n", ID_transmit);  //ID Ausgabe
  strcat(XML, buf);
  //send Botschaft an XML
  sprintf(buf, "<BO0>%d</BO0>\n", Botschaft_transmit);  //Botschaft Ausgabe
  strcat(XML, buf);
  
  xSemaphoreTake(can_receive_read_sem, portMAX_DELAY);

  //send receive flag an XML
  sprintf(buf, "<FL1>%d</FL1>\n", receive_data_ready_flag);  //Flag für Ausgabe an Konsole Ausgabe
  strcat(XML, buf);
  //send erhaltene CAN-ID an XML
  sprintf(buf, "<ID1>%s</ID1>\n", ID_receive);  //ID Ausgabe
  strcat(XML, buf);
  //send erhaltene Botschaft an XML
  sprintf(buf, "<BO1>%s</BO1>\n", Botschaft_receive); //Botschaft Ausgabe
  strcat(XML, buf);
  receive_data_ready_flag = 0;  //Ausgabe von erhaltenen Daten an Konsole beenden

  xSemaphoreGive(can_receive_write_sem);
  
  // show Button0 status
  if (BUTTON0) 
  {
    strcat(XML, "<BUTTON>1</BUTTON>\n");
  }
  else 
  {
    strcat(XML, "<BUTTON>0</BUTTON>\n");
  }


  strcat(XML, "</Data>\n");
  
  //Serial.println(XML); //Falls der gesendete XML im Serial Monitor gezeigt werden soll->Kommentar entfernen
  server.send(200, "text/xml", XML);
}

void printWifiStatus(void) 
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}