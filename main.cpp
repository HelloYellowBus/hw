// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------
#ifndef MBED_TEST_MODE

#include "http_request.h"
#include "mbed.h"
#include "simple-mbed-cloud-client.h"
#include "FATFileSystem.h"
#include "LittleFileSystem.h"
#include "GroveGPS.h" //gps
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"

char* thingspark_URL = "http://api.thingspark.kr";
char* thingspark_APIkey = "5aoWPhcOCnfWWvjr";
char urlBuffer[256];

// Default network interface object. 
NetworkInterface *net = NetworkInterface::get_default_instance();

// Default block device available on the target board
BlockDevice *bd = BlockDevice::get_default_instance();

#if COMPONENT_SD || COMPONENT_NUSD
// Use FATFileSystem for SD card type blockdevices
FATFileSystem fs("fs");
#else
// Use LittleFileSystem for non-SD block devices to enable wear leveling and other functions
LittleFileSystem fs("fs");
#endif

// static const ConnectorClientEndpointInfo* endpointInfo;
#define SENSORS_POLL_INTERVAL 1.0
#define SEND_ALL_SENSORS

// Default LED to use for PUT/POST example
// DigitalOut led(LED1, 0);

 //pin set
static DevI2C devI2c(PB_11,PB_10);
static HTS221Sensor sen_hum_temp(&devI2c);
static LPS22HBSensor sen_press_temp(&devI2c);

GroveGPS gps(D10, D4);
AnalogIn adc_temp(ADC_TEMP);
DigitalIn btn_blue(D11);
DigitalIn btn_red(D12);


// An event queue is a very useful structure to debounce information between contexts (e.g. ISR and normal threads)
// This is great because things such as network operations are illegal in ISR, so updating a resource in a button's fall() function is not allowed
// EventQueue eventQueue;

// void press_blue() {
//     printf("blue button pressed, ");
//     cnt = cnt + 1 ;
//     printf("count is %d\n", cnt);
//     wait(0.1);
// }

void sensors_init() {
    // uint8_t id1, id2;

    printf ("\nSensors init\n");
    // Initialize sensors
    sen_hum_temp.init(NULL);
    sen_press_temp.init(NULL);
 
    /// Call sensors enable routines
    sen_hum_temp.enable();
    sen_press_temp.enable();
}


// sensors_update 
void sensors_update(float people){
    printf("Sensor_update");
    printf("PeopleNm: %f", people);
    float temp1_value, temp2_value, temp3_value, humid_value, pressure_value = 0.0;
    char latBuffer[16], lonBuffer[16];
    // int ck = btn_blue.read();
    // int v = cnt;

    sen_hum_temp.get_humidity(&humid_value);
    sen_hum_temp.get_temperature(&temp1_value);
    sen_press_temp.get_pressure(&pressure_value);
    sen_press_temp.get_temperature(&temp2_value);
    temp3_value = adc_temp.read()*100;

    // printf("ADC temp:     %5.4f C,  vref:      5 V         \n", temp3_value);
    // printf("HTS221 temp:  %7.3f C,  humidity: %7.2f %%         \n", temp1_value, humid_value);
    // printf("LPS22HB temp: %7.3f C,  pressure: %7.2f mbar       \n", temp2_value, pressure_value);
    // printf("%d %d \n", v, ck);
    
    gps.getLatitude(latBuffer);
    gps.getLongitude(lonBuffer);

    // printf("\r\nLatitude: %s\r\nLongitude: %s\r\n", latBuffer, lonBuffer);
    
    // 190104 9:00 send data to things park
    sprintf(urlBuffer, "%s/update?apiKey=%s&field1=%.1f&field2=%.1f&field3=%s&field4=%s&field5=%f", thingspark_URL, thingspark_APIkey, humid_value, temp3_value, 
        latBuffer, lonBuffer, people);
    // printf("[DEBUG] %s\n", urlBuffer);
    HttpRequest* get_req = new HttpRequest(net, HTTP_GET, urlBuffer);
        
    HttpResponse* get_res = get_req->send();
    if (!get_res) {
        printf("HttpRequest failed (error code %d)\n", get_req->get_error());
        // return 1;
    }

    delete get_req;
}

/**
 * Registration callback handler
 * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
 */
// void registered(const ConnectorClientEndpointInfo *endpoint) {
//     printf("Registered to Pelion Device Management. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
//     endpointInfo = endpoint;
// }

int main(void) {
#if MBED_CONF_SERCOMM_TPB23_PROVIDE_DEFAULT == 1
    DigitalOut TPB23_RESET(A1);
    TPB23_RESET = 0;    /* 0: Standby 1: Reset */
    printf("\nSERCOM TPB23 Standby\n");
#elif MBED_CONF_QUECTEL_BG96_PROVIDE_DEFAULT == 1
    DigitalOut BG96_RESET(D7);
    DigitalOut BG96_PWRKEY(D9);
 
    BG96_RESET = 1;
    BG96_PWRKEY = 1;
    wait_ms(200);
 
    BG96_RESET = 0;
    BG96_PWRKEY = 0;
    wait_ms(300);
 
    BG96_RESET = 1;
    wait_ms(5000);
    printf("\nQUECTEL BG96 Standby\n");
#endif
    int storage_status = fs.mount(bd);
    if (storage_status != 0) {
        printf("Storage mounting failed.\n");
    }

    sensors_init();

    // Connect to the Internet (DHCP is expected to be on)
    printf("Connecting to the network using the default network interface...\n");
    net = NetworkInterface::get_default_instance();
    nsapi_error_t net_status = NSAPI_ERROR_NO_CONNECTION;
    while ((net_status = net->connect()) != NSAPI_ERROR_OK) {
        printf("Unable to connect to network (%d). Retrying...\n", net_status);
    }
    printf("Connected to the network successfully. IP address: %s\n", net->get_ip_address());
    printf("Initializing Pelion Device Management Client...\n");

    // The timer fires on an interrupt context, but debounces it to the eventqueue, so it's safe to do network operations
    // Ticker timer;?=
    // timer.attach(eventQueue.event(&button_press), 5.0);
    // EventQueue eventQueue;

    
    // timer.attach(eventQueue.event(&sensors_update), SENSORS_POLL_INTERVAL);
    printf("\r\n ----- Finish Read Sensor -------\n");
    
    // printf("Simulating button press every 5 seconds...\n");
    // btn_blue.rise(&press_blue);
    int blue_state, red_state = 0;
    int pre_blue_state, pre_red_state = 0;
    int blue_cnt=0;
    int red_cnt=0;
    while(1) {

      blue_state = btn_blue.read();
      red_state = btn_red.read();

      printf("blue_state: %d blue_cnt: %d\r\n",blue_state, blue_cnt);
      printf("red_state: %d  red_cnt: %d\r\n",red_state, red_cnt);

        if(blue_state == 1){
            if(pre_blue_state == 0){
                wait(0.01);
                pre_blue_state = 1;
            }
        }

        if(blue_state == 0){
            if(pre_blue_state == 1){
                blue_cnt ++ ;
                wait(0.01);
                pre_blue_state = 0;
            }
        } 

        if(red_state == 1){
            if(pre_red_state == 0){
                wait(0.01);
                pre_red_state = 1;
            }
        }

        if(red_state == 0){
            if(pre_red_state == 1){
                red_cnt ++ ;
                wait(0.01);
                pre_red_state = 0;
            }
        }
      printf("\r\nblue_cnt : %d,  red_cnt : %d\r\n", blue_cnt, red_cnt);
      sensors_update(float(blue_cnt-red_cnt));
      wait(0.01);
    };
    // You can easily run the eventQueue in a separate thread if required
    // eventQueue.dispatch_forever();
}

#endif /* MBED_TEST_MODE */