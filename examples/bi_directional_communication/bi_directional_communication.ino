// Trident WiFiPayload
// OpenROV 2018

// This example shows how one could setup bi-directional communication from the Android device on the surface to an Arduino-interfaced device and vice-versa.
// for this example, we will be using a fictional gripper arm

#include <WiFiPayload.h> // include header file. WiFiPayload folder must be stored in Arduino/libraries/

// WiFiPayload constructor (WiFi network name, WiFi network password, Trident's listening ip in string form, Trident's listening port as unsigned int).
WiFiPayload payload("ssid", "pswd", "10.10.10.10", 12345);

void setup() {

  // connects WiFiPayload to WiFi network, and prepares the outgoing and incoming message objects to accept data.
  payload.begin();

  // client identifier. If multiple payloads running on one trident, device names must be unique.
  payload.set_device_name("metal_detector");

}

/* For this example, we will assume that messages coming from Android device will be in the format:
 *  
 *    {"gripper_arm_command":0} for closing the arm, or
 *    {"gripper_arm_command":1} for opening the arm
 * 
 */

void loop() {

  // manages the connection to trident and performs all network communication. 
  // Always place before read() and write() functions (first function in loop() is best), and do not have other blocking functions inside the loop body
  payload.heartbeat();

  // are we connected to the WiFi?
  if(payload.is_connected()){

    // if a message was sent from the Android device
    // clears the incoming message from previous loop()
    if(payload.read() == 0){

      int cmnd  = -1;

      // if parsing the data field called "gripper_arm_command" successfully stored the value in cmnd
      if(payload.parse_data("gripper_arm_command", cmnd) == 0){

        if(cmnd == 0){

          tighten_grip();
          
        }
        if(cmnd == 1){

          release_grip();
          
        }
        
      }
      
    }

    // creates an object to store our metal detection data in the outgoing message object
    payload.add_object("gripper_arm_status");

    // take a metal detector reading
    int current_status = gripper_arm_status();

    // add a key ("reading 1") value (salinity) pair to the object called "salinity_data".
    payload.add_to_object("gripper_arm_status", "status", current_status);

    // write the outgoing message object to the Android device via Trident. 
    // This call clears the outgoing message.
    payload.write();

    /* The written message will be sent as such:
     * 
     * {"metal_detection_object":{"reading_1": 0}}
     * 
     */
    
  }

}

void tighten_grip(){

  // INSERT CODE
  
  return;
}

void release_grip(){
  
  // INSERT CODE

  return;
}

int gripper_arm_status(){

  //INSERT CODE
  
  return 0;
}

