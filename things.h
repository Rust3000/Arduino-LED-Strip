#pragma once


//ArduinoIoTCloud inneholder funksjonene som gjør at
//Arduinoen kan kommunisere med Arduino IoT Cloud
#include <ArduinoIoTCloud.h>

//Inneholder funksjoner for å koble Arduinoen til WiFi
#include <Arduino_ConnectionHandler.h>

//Brukes til å konfigurere nettverkstilkoblingen
//for eksempel gjennom BLE eller Serial
#include <Arduino_NetworkConfigurator.h>


//Disse to filene inneholder forskjellige metoder
//for å konfigurere nettverkstilkoblingen

//BLEAgent brukes til å konfigurere nettverket via Bluetooth
#include "configuratorAgents/agents/BLEAgent.h"

//SerialAgent brukes til å konfigurere nettverket via Serial
#include "configuratorAgents/agents/SerialAgent.h"



// CALLBACKS
//-----------------

//Disse funksjonene ligger i hovedprogrammet
//og blir kalt automatisk når en Cloud-verdi endres

//Kalles når ledStrip endres
void onLedStripChange();

//Kalles når activeLeds endres
void onActiveLedsChange();

//Kalles når cycleMode endres
void onCycleModeChange();



// CLOUD VARIABLES
//-------------------

//Lager en variabel for LED-stripen i Arduino Cloud
//Denne kan blant annet inneholde farge, lysstyrke og av/på-status
CloudColoredLight ledStrip;

//Bestemmer hvor mange LEDer som skal være aktive
//Her starter programmet med 5 aktive LEDer
int activeLeds = 5;

//Bestemmer om vi skal bytte modus gjennom Arduino Cloud
//false betyr at den ikke er aktiv
bool cycleMode = false;



// NETWORK CONFIGURATION
//-----------------------

//KVStore brukes til å lagre informasjon om nettverkskonfigurasjonen
KVStore kvStore;

//Lager en BLE-agent som kan brukes til nettverksoppsett
BLEAgentClass BLEAgent;

//Lager en Serial-agent som kan brukes til nettverksoppsett
SerialAgentClass SerialAgent;

//Lager forbindelsen som Arduinoen bruker til WiFi
WiFiConnectionHandler ArduinoIoTPreferredConnection;

//Lager en NetworkConfigurator som styrer hvordan
//Arduinoen konfigurerer nettverkstilkoblingen
NetworkConfiguratorClass NetworkConfigurator(
  ArduinoIoTPreferredConnection
);



// CLOUD PROPERTIES
//--------------------------

//Denne funksjonen brukes til å starte opp
//og registrere alle Arduino Cloud-variablene
void initProperties() {

  
  // NETWORK AGENTS
  //------------

  //Legger til BLE som en måte å konfigurere nettverket på
  NetworkConfigurator.addAgent(BLEAgent);

  //Legger til Serial som en annen måte å konfigurere nettverket på
  NetworkConfigurator.addAgent(SerialAgent);

  //Forteller NetworkConfigurator hvor informasjon
  //om nettverket skal lagres
  NetworkConfigurator.setStorage(kvStore);

  //Kobler NetworkConfigurator sammen med Arduino Cloud
  ArduinoCloud.setConfigurator(NetworkConfigurator);


  
  // LED STRIP PROPERTY
  // ---------------------------------
  //Legger ledStrip til Arduino Cloud
  ArduinoCloud.addProperty(
    ledStrip,

    //READWRITE betyr at Arduinoen både kan lese
    //verdien fra Cloud og skrive en verdi til Cloud
    READWRITE,

    //ON_CHANGE betyr at callback-funksjonen
    //blir kalt når verdien endres
    ON_CHANGE,

    //Funksjonen som skal kjøres når ledStrip endres
    onLedStripChange
  );


 
  // ACTIVE LEDS PROPERTY
  // --------------------------------

  //Legger activeLeds til Arduino Cloud
  ArduinoCloud.addProperty(
    activeLeds,

    //Arduinoen kan både lese og skrive verdien
    READWRITE,

    //Callback-funksjonen kjøres når verdien endres
    ON_CHANGE,

    //Funksjonen som skal kjøres når activeLeds endres
    onActiveLedsChange
  );


  
  // CYCLE MODE PROPERTY
  // --------------------------------------------------

  //Legger cycleMode til Arduino Cloud
  ArduinoCloud.addProperty(
    cycleMode,

    //Arduinoen kan både lese og skrive verdien
    READWRITE,

    //Callback-funksjonen kjøres når verdien endres
    ON_CHANGE,

    //Funksjonen som skal kjøres når cycleMode endres
    onCycleModeChange
  );
}
