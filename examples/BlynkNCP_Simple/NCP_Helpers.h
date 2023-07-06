#if !defined(BLYNK_FIRMWARE_TYPE) && defined(BLYNK_TEMPLATE_ID)
  #define BLYNK_FIRMWARE_TYPE BLYNK_TEMPLATE_ID
#endif

#if !defined(BLYNK_TEMPLATE_ID) || !defined(BLYNK_TEMPLATE_NAME)
  #error "Please specify your BLYNK_TEMPLATE_ID and BLYNK_TEMPLATE_NAME"
#endif

#define BLYNK_PARAM_KV(k, v) k "\0" v "\0"

#include <BlynkRpcClient.h>

#if defined(SerialNCP)
  // OK, use it
  void ncpInitialize() {
    // Power-up NCP, if needed
  }
  void ncpConfigure() {}
#elif defined(ARDUINO_NANO_RP2040_CONNECT) && defined(__MBED__)
  #define SerialNCP   SerialNina
  void ncpInitialize() {
    pinMode(NINA_RESETN, OUTPUT);
    digitalWrite(NINA_RESETN, HIGH);
  }

  void ncpConfigure() {
    rpc_hw_initRGB(27, 25, 26, true);
    rpc_hw_setLedBrightness(128);
  }
#else
  #error "SerialNCP is not defined"
#endif

#include <BlynkRpcInfraArduino.h>

// Embed the info tag into the MCU firmware binary
// This structure is used to identify the firmware type and version during the OTA upgrade
volatile const char firmwareTag[] = "blnkinf\0"
    BLYNK_PARAM_KV("mcu"    , BLYNK_FIRMWARE_VERSION)
    BLYNK_PARAM_KV("fw-type", BLYNK_FIRMWARE_TYPE)
    BLYNK_PARAM_KV("build"  , BLYNK_FIRMWARE_BUILD_TIME)
    BLYNK_PARAM_KV("blynk"  , BLYNK_RPC_LIB_VERSION)
    "\0";

bool ncpWaitResponse(uint32_t timeout = 10000) {
  const uint32_t tbeg = millis();
  while (millis() - tbeg < timeout) {
    if (RPC_STATUS_OK == rpc_ncp_ping()) {
      SerialDbg.println("Blynk.NCP response OK");
      return true;
    }
  }
  SerialDbg.println("NCP not responding");
  return false;
}

const char* ncpGetStateString(uint8_t state) {
  switch (state) {
  case BLYNK_STATE_IDLE             : return "Idle";
  case BLYNK_STATE_CONFIG           : return "Configuration";
  case BLYNK_STATE_CONNECTING_NET   : return "Connecting Network";
  case BLYNK_STATE_CONNECTING_CLOUD : return "Connecting Cloud";
  case BLYNK_STATE_CONNECTED        : return "Connected";

  case BLYNK_STATE_NOT_INITIALIZED  : return "Not Initialized";
  case BLYNK_STATE_OTA_UPGRADE      : return "NCP Upgrade";
  case BLYNK_STATE_ERROR            : return "Error";

  default                           : return "Unknown";
  }
}
