#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "GPIO.hpp"

// Define GPIO pins for each light
#define RED_PIN    5
#define YELLOW_PIN 6
#define GREEN_PIN  7

// Define state timing values in ms
#define RED_DURATION          4000
#define RED_YELLOW_DURATION   4000
#define GREEN_DURATION        4000
#define GREEN_BLINK_DURATION  4000
#define YELLOW_DURATION       4000
#define BLINK_INTERVAL        1000

// Traffic light states enum
enum TrafficLightState {
    STATE_RED,
    STATE_RED_YELLOW,
    STATE_GREEN,
    STATE_GREEN_BLINKING,
    STATE_YELLOW
};

// Create GPIO objects for each light
static pico_cpp::GPIO_Pin redLed(RED_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin yellowLed(YELLOW_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin greenLed(GREEN_PIN, pico_cpp::PinType::Output);

// Queue for state transitions
QueueHandle_t stateQueue;

// Function prototypes
void trafficLightControlTask(void *pvParameters);
void stateTransitionTask(void *pvParameters);
void setLightState(TrafficLightState state);

// Main function
int main() {
    stdio_init_all();
    printf("Traffic Light Controller\n");

    // Create queue for state transitions
    stateQueue = xQueueCreate(5, sizeof(TrafficLightState));
    
    if (stateQueue == NULL) {
        printf("Failed to create state queue!\n");
        return 1;
    }

    // Create the control task
    xTaskCreate(
        trafficLightControlTask,
        "LightControl",
        1024,
        NULL,
        1,
        NULL
    );

    // Create the state transition task
    xTaskCreate(
        stateTransitionTask,
        "StateTransition",
        1024,
        NULL,
        2,
        NULL
    );

    // Start the scheduler
    vTaskStartScheduler();

    // Should never reach here
    for (;;);
    return 0;
}

// Set the physical state of the LEDs
void setLightState(TrafficLightState state) {
    // Turn off all lights first
    redLed.set_low();
    yellowLed.set_low();
    greenLed.set_low();
    
    // Then set the appropriate lights based on state
    switch (state) {
        case STATE_RED:
            redLed.set_high();
            break;
        case STATE_RED_YELLOW:
            redLed.set_high();
            yellowLed.set_high();
            break;
        case STATE_GREEN:
            greenLed.set_high();
            break;
        case STATE_GREEN_BLINKING:
            // Blinking handled in the control task
            break;
        case STATE_YELLOW:
            yellowLed.set_high();
            break;
    }
}

// Controls the actual light states
void trafficLightControlTask(void *pvParameters) {
    TrafficLightState currentState = STATE_RED;
    TrafficLightState nextState;
    
    // Send initial state
    if (xQueueSend(stateQueue, &currentState, 0) != pdPASS) {
        printf("Failed to send initial state\n");
    }
    
    for (;;) {
        // Get next state from queue
        if (xQueueReceive(stateQueue, &currentState, portMAX_DELAY) == pdPASS) {
            printf("Changing to state: %d\n", currentState);
            
            switch (currentState) {
                case STATE_RED:
                    setLightState(currentState);
                    vTaskDelay(pdMS_TO_TICKS(RED_DURATION));
                    nextState = STATE_RED_YELLOW;
                    break;
                    
                case STATE_RED_YELLOW:
                    setLightState(currentState);
                    vTaskDelay(pdMS_TO_TICKS(RED_YELLOW_DURATION));
                    nextState = STATE_GREEN;
                    break;
                    
                case STATE_GREEN:
                    setLightState(currentState);
                    vTaskDelay(pdMS_TO_TICKS(GREEN_DURATION));
                    nextState = STATE_GREEN_BLINKING;
                    break;
                    
                case STATE_GREEN_BLINKING:
                    // Handle green blinking
                    for (int i = 0; i < (GREEN_BLINK_DURATION / (BLINK_INTERVAL * 2)); i++) {
                        greenLed.set_high();
                        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
                        greenLed.set_low();
                        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
                    }
                    nextState = STATE_YELLOW;
                    break;
                    
                case STATE_YELLOW:
                    setLightState(currentState);
                    vTaskDelay(pdMS_TO_TICKS(YELLOW_DURATION));
                    nextState = STATE_RED;
                    break;
            }
            
            // Send the next state to the queue
            xQueueSend(stateQueue, &nextState, 0);
        }
    }
}

// Diagnostic task for state transitions
void stateTransitionTask(void *pvParameters) {
    TrafficLightState prevState = STATE_YELLOW; // Initialize to something
    TrafficLightState currState;
    
    for (;;) {
        // Peek at the current state without removing it
        if (xQueuePeek(stateQueue, &currState, pdMS_TO_TICKS(500)) == pdPASS) {
            if (currState != prevState) {
                printf("State transition: %d -> %d\n", prevState, currState);
                prevState = currState;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
