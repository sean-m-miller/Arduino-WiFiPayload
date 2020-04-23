// This example shows how one could setup single-direction communication from an Arduino-interfaced device to the Android device as the surface.
// for this example, we will be using a fictional metal detector

#include <WiFiPayload.h> // include header file. WiFiPayload folder must be stored in Arduino/libraries/

// WiFiPayload constructor (WiFi network name, WiFi network password, server's listening ip in string form, server's listening port as unsigned int).
WiFiPayload payload("ssid", "pswd", "10.10.10.10", 12345);

void setup() {

  // connects WiFiPayload to WiFi network, and prepares the outgoing and incoming message objects to accept data.
  payload.begin();

  // client identifier. If multiple payloads running on one server, device names must be unique.
  payload.set_device_name("metal_detector");

}

void loop() {

  // manages the connection to server and performs all network communication. 
  // Always place before read() and write() functions (first function in loop() is best), and do not have other blocking functions inside the loop body
  payload.heartbeat();

  // are we connected to the WiFi?
  if(payload.is_connected()){

    // creates an object to store our metal detection data in the outgoing message object
    payload.add_object("metal_detection_object");

    // take a metal detector reading
    int metal_detection = read_from_metal_detector();

    // add a key ("reading 1") value (metal_detection) pair to the object called "metal_detection_object".
    payload.add_to_object("metal_detection_object", "reading 1", metal_detection);

    // write the outgoing message object to the Android device via server. 
    // This call clears the outgoing message.
    payload.write();

    /* The written message will be sent as such:
     * 
     * {"metal_detection_object":{"reading_1": 0}}
     * 
     */
    
  }

}

int read_from_metal_detector(){

// INSERT METAL DETECTOR CODE
  
  return 0;
}

