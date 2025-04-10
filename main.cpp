#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "GPIO.hpp"

// Define GPIO pins for each light
#define RED_PIN 2
#define YELLOW_PIN 3
#define GREEN_PIN 4

// Define task priority levels
#define HIGH_PRIORITY 2
//#define MEDIUM_PRIORITY 1
#define LOW_PRIORITY 1

// Define state timing values in ms
#define STATE_DURATION 4000
#define BLINK_INTERVAL 500

// Traffic light states enum
enum TrafficLightState {
    STATE_RED,
    STATE_RED_YELLOW,
    STATE_GREEN,
    STATE_GREEN_BLINKING,
    STATE_YELLOW,
    STATE_YELLOW_BLINKING
};

// Create GPIO objects for each light
static pico_cpp::GPIO_Pin redLed(RED_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin yellowLed(YELLOW_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin greenLed(GREEN_PIN, pico_cpp::PinType::Output);

// Task handles for each state task
TaskHandle_t redTaskHandle = NULL;
TaskHandle_t redYellowTaskHandle = NULL;
TaskHandle_t greenTaskHandle = NULL;
TaskHandle_t greenBlinkTaskHandle = NULL;
TaskHandle_t yellowTaskHandle = NULL;

// State functions to set the lights
void setStateRed() {
    redLed.set_high();
    yellowLed.set_low();
    greenLed.set_low();
}

void setStateRedYellow() {
    redLed.set_high();
    yellowLed.set_high();
    greenLed.set_low();
}

void setStateGreen() {
    redLed.set_low();
    yellowLed.set_low();
    greenLed.set_high();
}

void setStateGreenBlinking() {
    redLed.set_low();
    yellowLed.set_low();
    for (int i = 0; i < 4; i++) {
        greenLed.set_high();
        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
        greenLed.set_low();
        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
    }
}

void setStateYellow() {
    redLed.set_low();
    yellowLed.set_high();
    greenLed.set_low();
}

void setStateYellowBlinking() {
    redLed.set_low();
    greenLed.set_low();
    for (;;) {
        yellowLed.set_high();
        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
        yellowLed.set_low();
        vTaskDelay(pdMS_TO_TICKS(BLINK_INTERVAL));
    }
}

// Task functions for each state
void redTask(void *pvParameters) {
    for (;;) {
        setStateRed();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION));
        vTaskPrioritySet(redYellowTaskHandle, HIGH_PRIORITY);
        vTaskPrioritySet(NULL, LOW_PRIORITY);
    }
}

void redYellowTask(void *pvParameters) {
    for (;;) {
        setStateRedYellow();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION));
        vTaskPrioritySet(greenTaskHandle, HIGH_PRIORITY);
        vTaskPrioritySet(NULL, LOW_PRIORITY);
    }
}

void greenTask(void *pvParameters) {
    for (;;) {
        setStateGreen();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION));
        vTaskPrioritySet(greenBlinkTaskHandle, HIGH_PRIORITY);
        vTaskPrioritySet(NULL, LOW_PRIORITY);
    }
}

void greenBlinkTask(void *pvParameters) {
    for (;;) {
        setStateGreenBlinking();
        vTaskPrioritySet(yellowTaskHandle, HIGH_PRIORITY);
        vTaskPrioritySet(NULL, LOW_PRIORITY);
    }
}

void yellowTask(void *pvParameters) {
    for (;;) {
        setStateYellow();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION));
        vTaskPrioritySet(redTaskHandle, HIGH_PRIORITY);
        vTaskPrioritySet(NULL, LOW_PRIORITY);
    }
}

// Main function to create tasks and start the scheduler
int main() {
    stdio_init_all();

    // Create tasks with initial priorities
    xTaskCreate(redTask, "Red", 1024, NULL, HIGH_PRIORITY, &redTaskHandle);
    xTaskCreate(redYellowTask, "RedYellow", 1024, NULL, LOW_PRIORITY, &redYellowTaskHandle);
    xTaskCreate(greenTask, "Green", 1024, NULL, LOW_PRIORITY, &greenTaskHandle);
    xTaskCreate(greenBlinkTask, "GreenBlink", 1024, NULL, LOW_PRIORITY, &greenBlinkTaskHandle);
    xTaskCreate(yellowTask, "Yellow", 1024, NULL, LOW_PRIORITY, &yellowTaskHandle);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    // Fallback if the scheduler encounters an error
    setStateYellowBlinking()        
}
