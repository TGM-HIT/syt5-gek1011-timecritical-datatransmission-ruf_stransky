#ifndef TRAFFIC_LIGHT_HPP
#define TRAFFIC_LIGHT_HPP

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "FreeRTOS.h"
#include "task.h"

// LED pin definitions
#define LED_RED_PIN     2
#define LED_YELLOW_PIN  3
#define LED_GREEN_PIN   4

// SPI pin definitions
#define SPI_MOSI_PIN    19
#define SPI_SCK_PIN     18
#define SPI_CS_PIN      17

// Traffic light state codes as specified in the task
enum class LightState : uint8_t {
    RED             = 0b1110,  // 1-1-1-0
    RED_YELLOW      = 0b1101,  // 1-1-0-1
    GREEN           = 0b0010,  // 0-0-1-0
    GREEN_BLINKING  = 0b0101,  // 0-1-0-1
    YELLOW          = 0b1000,  // 1-0-0-0
    YELLOW_BLINKING = 0b0001   // 0-0-0-1 (error state)
};

class TrafficLight {
private:
    // Timing parameters (ms)
    static constexpr uint32_t RED_TIME = 3000;
    static constexpr uint32_t RED_YELLOW_TIME = 1000;
    static constexpr uint32_t GREEN_TIME = 3000;
    static constexpr uint32_t GREEN_BLINKING_TIME = 1000;
    static constexpr uint32_t YELLOW_TIME = 1000;
    static constexpr uint32_t BLINK_INTERVAL = 250;
    
    LightState currentState;
    bool spiConnectionOk;
    TickType_t lastSpiActivity;
    
    // Set LEDs based on current state
    void setLeds();
    
    // Send state via SPI
    bool sendStateSpi();

public:
    TrafficLight();
    
    // Initialize hardware
    void init();
    
    // Task functions
    static void trafficLightTask(void* pvParameters);
    static void spiMonitorTask(void* pvParameters);
    
    // State handling
    void setState(LightState state);
    LightState getState() const;
    
    // Connection status
    bool isSpiConnectionOk() const;
    void setSpiConnectionOk(bool ok);
    
    // Update SPI activity timestamp
    void updateSpiActivity();
    TickType_t getLastSpiActivity() const;
};

#endif // TRAFFIC_LIGHT_HPP
