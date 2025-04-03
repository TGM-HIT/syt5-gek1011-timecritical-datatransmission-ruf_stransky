#include "traffic_light.hpp"
#include <stdio.h>
#include "hardware/gpio.h"

// Static instance to be accessible from C-style FreeRTOS task functions
static TrafficLight* trafficLightInstance = nullptr;

TrafficLight::TrafficLight() : 
    currentState(LightState::RED),
    spiConnectionOk(true),
    lastSpiActivity(0)
{
    trafficLightInstance = this;
}

void TrafficLight::init() {
    // Initialize traffic light LEDs
    gpio_init(LED_RED_PIN);
    gpio_init(LED_YELLOW_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_YELLOW_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    
    // Turn off all LEDs initially
    gpio_put(LED_RED_PIN, 0);
    gpio_put(LED_YELLOW_PIN, 0);
    gpio_put(LED_GREEN_PIN, 0);
    
    // Initialize SPI
    spi_init(spi0, 1000 * 1000); // 1MHz clock
    
    // Configure SPI GPIOs
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
    
    // Chip select is GPIO controlled
    gpio_init(SPI_CS_PIN);
    gpio_set_dir(SPI_CS_PIN, GPIO_OUT);
    gpio_put(SPI_CS_PIN, 1); // Deselect by default
    
    // Initialize SPI activity timestamp
    lastSpiActivity = xTaskGetTickCount();
    
    printf("Traffic Light initialized\n");
}

void TrafficLight::setLeds() {
    // Reset all LEDs
    gpio_put(LED_RED_PIN, 0);
    gpio_put(LED_YELLOW_PIN, 0);
    gpio_put(LED_GREEN_PIN, 0);
    
    // Set LEDs based on current state
    switch (currentState) {
        case LightState::RED:
            gpio_put(LED_RED_PIN, 1);
            break;
        case LightState::RED_YELLOW:
            gpio_put(LED_RED_PIN, 1);
            gpio_put(LED_YELLOW_PIN, 1);
            break;
        case LightState::GREEN:
            gpio_put(LED_GREEN_PIN, 1);
            break;
        case LightState::GREEN_BLINKING:
            // Blinking will be handled in the task
            break;
        case LightState::YELLOW:
            gpio_put(LED_YELLOW_PIN, 1);
            break;
        case LightState::YELLOW_BLINKING:
            // Blinking will be handled in the task
            break;
    }
}

bool TrafficLight::sendStateSpi() {
    uint8_t code = static_cast<uint8_t>(currentState);
    
    // Select SPI device
    gpio_put(SPI_CS_PIN, 0);
    
    // Send the state code
    spi_write_blocking(spi0, &code, 1);
    
    // Deselect SPI device
    gpio_put(SPI_CS_PIN, 1);
    
    // Update timestamp of successful SPI communication
    updateSpiActivity();
    
    printf("SPI sent state: %02X\n", code);
    return true;
}

void TrafficLight::setState(LightState state) {
    currentState = state;
    printf("State changed to: %d\n", static_cast<int>(state));
}

LightState TrafficLight::getState() const {
    return currentState;
}

bool TrafficLight::isSpiConnectionOk() const {
    return spiConnectionOk;
}

void TrafficLight::setSpiConnectionOk(bool ok) {
    spiConnectionOk = ok;
}

void TrafficLight::updateSpiActivity() {
    lastSpiActivity = xTaskGetTickCount();
}

TickType_t TrafficLight::getLastSpiActivity() const {
    return lastSpiActivity;
}

// FreeRTOS task for traffic light control
void TrafficLight::trafficLightTask(void* pvParameters) {
    // Get the traffic light instance
    TrafficLight* light = trafficLightInstance;
    bool ledState = false;
    
    for (;;) {
        if (light->isSpiConnectionOk()) {
            // Normal operation cycle
            
            // RED
            light->setState(LightState::RED);
            light->setLeds();
            light->sendStateSpi();
            vTaskDelay(pdMS_TO_TICKS(RED_TIME));
            
            if (!light->isSpiConnectionOk()) continue;
            
            // RED_YELLOW
            light->setState(LightState::RED_YELLOW);
            light->setLeds();
            light->sendStateSpi();
            vTaskDelay(pdMS_TO_TICKS(RED_YELLOW_TIME));
            
            if (!light->isSpiConnectionOk()) continue;
            
            // GREEN
            light->setState(LightState::GREEN);
            light->setLeds();
            light->sendStateSpi();
            vTaskDelay(pdMS_TO_TICKS(GREEN_TIME));
            
            if (!light->isSpiConnectionOk()) continue;
            
            // GREEN_BLINKING
            light->setState(LightState::GREEN_BLINKING);
            light->sendStateSpi();
            
            // Handle green blinking
            TickType_t blinkStart = xTaskGetTickCount();
            while ((xTaskGetTickCount() - blinkStart) < pdMS_TO_TICKS(GREEN_BLINKING_TIME)) {
                if (!light->isSpiConnectionOk()) break;
                
                ledState = !ledState;
                gpio_put(LED_GREEN_PIN, ledState);
                vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
            }
            
            if (!light->isSpiConnectionOk()) continue;
            
            // YELLOW
            light->setState(LightState::YELLOW);
            light->setLeds();
            light->sendStateSpi();
            vTaskDelay(pdMS_TO_TICKS(YELLOW_TIME));
            
        } else {
            // Error state - yellow blinking
            light->setState(LightState::YELLOW_BLINKING);
            light->sendStateSpi();
            
            // Handle yellow blinking
            ledState = !ledState;
            gpio_put(LED_YELLOW_PIN, ledState);
            vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
        }
    }
}

// FreeRTOS task for monitoring SPI connection
void TrafficLight::spiMonitorTask(void* pvParameters) {
    // Get the traffic light instance
    TrafficLight* light = trafficLightInstance;
    
    for (;;) {
        // Check if more than 60ms passed since last SPI activity
        if ((xTaskGetTickCount() - light->getLastSpiActivity()) > pdMS_TO_TICKS(60)) {
            if (light->isSpiConnectionOk()) {
                printf("SPI connection lost! No activity for >60ms\n");
                light->setSpiConnectionOk(false);
            }
        } else {
            if (!light->isSpiConnectionOk()) {
                printf("SPI connection restored\n");
                light->setSpiConnectionOk(true);
            }
        }
        
        // Check every 10ms
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
