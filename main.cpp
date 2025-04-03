#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "traffic_light.hpp"

int main() {
    // Initialize standard library
    stdio_init_all();
    
    printf("\nTraffic Light SPI Controller\n");
    printf("----------------------------\n");
    
    // Create and initialize traffic light
    TrafficLight trafficLight;
    trafficLight.init();
    
    // Create FreeRTOS tasks
    xTaskCreate(
        TrafficLight::trafficLightTask,
        "TrafficLightTask",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
    
    xTaskCreate(
        TrafficLight::spiMonitorTask,
        "SPIMonitorTask",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 2, // Higher priority for monitoring
        NULL
    );
    
    // Start the scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    for (;;) {
        // Fallback loop if scheduler fails
    }
    
    return 0;
}
