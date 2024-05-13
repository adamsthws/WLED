#pragma once

#include "wled.h"

class InternalTemperatureUsermod : public Usermod
{

private:
  unsigned long loopInterval = 10000;
  unsigned long lastTime = 0;
  bool isEnabled = false;
  float temperature = 0;
  float activationThreshold = 90.0;     // Default temperature threshold
  float resetThreshold = 89.0;          // Reactivation temperature threshold
  //float reactivationDifference = 1.0; // Reactivation temperature threshold
  //int lastPreset = 0;                 // Variable to store the last active preset
  int presetToActivate = 99;            // Default preset setting (-1 = disabled)
  bool isAboveThreshold = false;        // Flag to track if the high temperature preset is currently active

  static const char _name[];
  static const char _enabled[];
  static const char _loopInterval[];
  static const char _activationThreshold[];
  static const char _resetThreshold[];
  static const char _presetToActivate[];

  // any private methods should go here (non-inline method should be defined out of class)
  void publishMqtt(const char *state, bool retain = false); // example for publishing MQTT message

public:
  void setup()
  {
  //resetThreshold = activationThreshold - reactivationDifference;
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!isEnabled || strip.isUpdating() || millis() - lastTime <= loopInterval)
      return;

    lastTime = millis();

// Measure the temperature
#ifdef ESP8266 // ESP8266
    // does not seem possible
    temperature = -1;
#elif defined(CONFIG_IDF_TARGET_ESP32S2) // ESP32S2
    temperature = -1;
#else                                    // ESP32 ESP32S3 and ESP32C3
    temperature = roundf(temperatureRead() * 10) / 10;
#endif

    // Check if temperature has gone above the threshold
    if (temperature >= activationThreshold) {
      // Update the state flag if not already set
      if (!isAboveThreshold){
        isAboveThreshold = true;
        }
      // Activate the 'over-threshold' preset if it's not already active
      if (presetToActivate != -1 && currentPreset != presetToActivate) {
        //lastPreset = currentPreset;      // Save the current preset to allow re-activation later
        saveTemporaryPreset();             // Save the current preset to allow re-activation later
        applyPreset(presetToActivate);     // Activate the high temperature preset
        }
      }
    // Check if temperature is back below the threshold
    else if (temperature <= resetThreshold) {
      // Update the state flag if not already set
      if (isAboveThreshold){
        isAboveThreshold = false;
        }
      // Revert back to the original preset
      if (currentPreset == presetToActivate){
        applyTemporaryPreset();
        //applyPreset(lastPreset);
        }
      }

#ifndef WLED_DISABLE_MQTT
    if (WLED_MQTT_CONNECTED)
    {
      char array[10];
      snprintf(array, sizeof(array), "%f", temperature);
      publishMqtt(array);
    }
#endif
  }

  void addToJsonInfo(JsonObject &root)
  {
    if (!isEnabled)
      return;

    // if "u" object does not exist yet we need to create it
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");

    JsonArray userTempArr = user.createNestedArray(FPSTR(_name));
    userTempArr.add(temperature);
    userTempArr.add(F(" °C"));

    // if "sensor" object does not exist yet we need to create it
    JsonObject sensor = root[F("sensor")];
    if (sensor.isNull())
      sensor = root.createNestedObject(F("sensor"));

    JsonArray sensorTempArr = sensor.createNestedArray(FPSTR(_name));
    sensorTempArr.add(temperature);
    sensorTempArr.add(F("°C"));
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = isEnabled;
    top[FPSTR(_loopInterval)] = loopInterval;
    top[FPSTR(_activationThreshold)] = activationThreshold;
    top[FPSTR(_resetThreshold)] = resetThreshold;
    top[FPSTR(_presetToActivate)] = presetToActivate;
  }

    // Append useful info to the usermod settings gui
    void appendConfigData()
    {
    // Display 'ms' next to the 'Loop Interval' setting
    oappend(SET_F("addInfo('Internal Temperature:Loop Interval', 1, 'ms');"));
    // Display '°C' next to the 'Activation Threshold' setting
    oappend(SET_F("addInfo('Internal Temperature:Activation Threshold', 1, '°C');"));
    // Display '-1 = disabled' next to the 'Preset To Activate' setting
    oappend(SET_F("addInfo('Internal Temperature:Preset To Activate', 1, '-1 = disabled');"));
    }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top[FPSTR(_enabled)], isEnabled);
    configComplete &= getJsonValue(top[FPSTR(_loopInterval)], loopInterval);
    configComplete &= getJsonValue(top[FPSTR(_presetToActivate)], presetToActivate);

    // Calculate the difference between activation and re-activation thresholds
    //float oldactivationThreshold = activationThreshold; // Store the old threshold value to compare
    configComplete &= getJsonValue(top[FPSTR(_activationThreshold)], activationThreshold); // Get activation threshold value
    configComplete &= getJsonValue(top[FPSTR(_resetThreshold)], resetThreshold);  // Get reset threshold value
    //if (activationThreshold != oldactivationThreshold) {    // If activation threshold changed...
    //        resetThreshold = activationThreshold - reactivationDifference;  // Calculate the new re-activation threshold
    //    }

    return configComplete;
  }

  uint16_t getId()
  {
    return USERMOD_ID_INTERNAL_TEMPERATURE;
  }
};

const char InternalTemperatureUsermod::_name[] PROGMEM = "Internal Temperature";
const char InternalTemperatureUsermod::_enabled[] PROGMEM = "Enabled";
const char InternalTemperatureUsermod::_loopInterval[] PROGMEM = "Loop Interval";
const char InternalTemperatureUsermod::_activationThreshold[] PROGMEM = "Activation Threshold";
const char InternalTemperatureUsermod::_presetToActivate[] PROGMEM = "Preset To Activate";
const char InternalTemperatureUsermod::_resetThreshold[] PROGMEM = "Reset Threshold";

void InternalTemperatureUsermod::publishMqtt(const char *state, bool retain)
{
#ifndef WLED_DISABLE_MQTT
  // Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED)
  {
    char subuf[64];
    strcpy(subuf, mqttDeviceTopic);
    strcat_P(subuf, PSTR("/mcutemp"));
    mqtt->publish(subuf, 0, retain, state);
  }
#endif
}

