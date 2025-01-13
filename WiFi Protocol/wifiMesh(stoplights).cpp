/**
 * @file wifiMesh(stoplights).cpp
 * @author Alessandro Ferrante (github@alessandroferrante.net)
 * @brief This file contains the implementation of a WiFi mesh network for simulating traffic lights using the painlessMesh library.
 * 
 * The program initializes a mesh network with a specified prefix, password, and port. It handles sending and receiving messages
 * to simulate the state of traffic lights (RED and GREEN) across multiple nodes in the mesh network.
 *
 * ? The main functionalities include:
 * - Initializing the mesh network.
 * - Sending broadcast messages to update traffic light states.
 * - Handling received messages to update the state of the traffic lights.
 * - Managing new connections in the mesh network.
 * - Updating the color of the traffic lights based on the current state.
 * 
 * ? Functions
 * - void sendMessage(): Sends a broadcast message to update the traffic light state.
 * - void updateState(): Updates the color of the traffic light based on the current state.
 * - void receivedCallback(uint32_t from, String &msg): Handles received messages and updates the traffic light state.
 * - void newConnectionCallback(uint32_t nodeId): Handles new connections in the mesh network.
 * - void setColor(int R, int G, int B): Sets the color of the RGB LED.
 * - void setup(): Initializes the mesh network and sets up the initial state.
 * - void loop(): Continuously updates the mesh network.
 * @version 0.1
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024
 */

#include <Arduino.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "alessandro"
#define   MESH_PASSWORD   "ferrante"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

void sendMessage() ; // Prototype so PlatformIO doesn't complain
void updateState();
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

uint32_t idFirstNode = 0;
String msg = "RED";
bool state = false;
void sendMessage() {
    mesh.sendBroadcast( msg );
    taskSendMessage.setInterval( TASK_SECOND * 5);
}

// Needed for painless library

int flag = 0; // for 1st snippet
void receivedCallback( uint32_t from, String &msg ) {

    //* 1st snippet to simulate 2 traffic lights
    /*
    if(msg.equals("RED")){
        state = true; // diverso da l'altro
        if (flag == 1){
            flag = 0;
            state = false;
        }else{
            flag = 1 ;
        } 
        updateState();
    }
    else if(msg.equals("GREEN")){
        state = false;

        if (flag == 2){
            flag = 0;
            state = true;
        }else{
            flag = 2 ;
        } 

        updateState();
    }
    */

    //* 2nd snippet to simulate 2 traffic lights
    /*
    if(msg.equals("RED") && state == false){
            state = true; // diverso da l'altro
            updateState();
    }
    else{
        state = false; 
        updateState();
    }
    */

    //* 3rd snippet to simulate 3 traffic lights
    
    if (from == idFirstNode){

    }
    else {
        if(msg.equals("RED") && state == false){
            state = true;
            updateState();
        }
        else{
            state = false; 
            updateState();
        }
    }
    
    Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

bool first = true;
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    
    // snippet to simulate 3 traffic lights
    if(first){
        idFirstNode = nodeId;
        first = false;
    }

    state = !state;
    updateState();
}

void setColor(int R,int G,int B){
  analogWrite(LED_RED, R);
  analogWrite(LED_GREEN, G);
  analogWrite(LED_BLUE, B);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Avvio del sistema...");
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(LED_RED,OUTPUT);
    pinMode(LED_GREEN,OUTPUT);   
    pinMode(LED_BLUE,OUTPUT);

    mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);

    userScheduler.addTask( taskSendMessage );
    taskSendMessage.enable();  
    updateState();
}

void updateState(){
    if (state == true){
        setColor(255, 0, 255); // GREEN
        msg = "GREEN";
    }
    else{
        setColor(0, 255, 255); // RED
        msg = "RED";
    }
}

void loop() {
    mesh.update();
}