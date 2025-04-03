#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "GPIO.hpp"

// Define GPIO pins for each light
#define RED_PIN    1
#define YELLOW_PIN 2
#define GREEN_PIN  3

// Define task priority levels
#define HIGH_PRIORITY 1
#define LOW_PRIORITY  0

// define the state time in ms
#define STATE_DURATION 4000

// Create GPIO objects for each light
static pico_cpp::GPIO_Pin redLight(RED_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin yellowLight(YELLOW_PIN, pico_cpp::PinType::Output);
static pico_cpp::GPIO_Pin greenLight(GREEN_PIN, pico_cpp::PinType::Output);

// Task handles for each state task
TaskHandle_t redTaskHandle = NULL;
TaskHandle_t redYellowTaskHandle = NULL;
TaskHandle_t greenTaskHandle = NULL;
TaskHandle_t greenBlinkTaskHandle = NULL;
TaskHandle_t yellowTaskHandle = NULL;

// State functions

void stateRed() {
    // Only red on
    redLight.set_high();
    yellowLight.set_low();
    greenLight.set_low();
}

void stateRedYellow() {
    // Red and Yellow on
    redLight.set_high();
    yellowLight.set_high();
    greenLight.set_low();
}

void stateGreen() {
    // Only green on
    redLight.set_low();
    yellowLight.set_low();
    greenLight.set_high();
}

void stateGreenBlinking() {
    redLight.set_low();
    yellowLight.set_low();
    for (int i = 0; i < 5; i++) {
        greenLight.set_high();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION/8));
        greenLight.set_low();
        vTaskDelay(pdMS_TO_TICKS(STATE_DURATION/8));
    }
}

void stateYellow() {
    // Only yellow on
    redLight.set_low();
    yellowLight.set_high();
    greenLight.set_low();
}

// Task functions: Each task represents one state

void redTask(void *pvParameters) {
    for (;;) {
        if (uxTaskPriorityGet(NULL) == HIGH_PRIORITY) {
            stateRed();
            vTaskDelay(pdMS_TO_TICKS(STATE_DURATION));
            vTaskPrioritySet(redYellowTaskHandle, HIGH_PRIORITY);
            vTaskPrioritySet(NULL, LOW_PRIORITY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void redYellowTask(void *pvParameters) {
    for (;;) {
        if (uxTaskPriorityGet(NULL) == HIGH_PRIORITY) {
            stateRedYellow();
            vTaskDelay(pdMS_TO_TICKS(STATE_DURATION)); // 4-second duration
            // Transition to next state: Green
            vTaskPrioritySet(greenTaskHandle, HIGH_PRIORITY);
            vTaskPrioritySet(NULL, LOW_PRIORITY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void greenTask(void *pvParameters) {
    for (;;) {
        if (uxTaskPriorityGet(NULL) == HIGH_PRIORITY) {
            stateGreen();
            vTaskDelay(pdMS_TO_TICKS(STATE_DURATION)); // 4-second duration
            // Transition to next state: Green blinking
            vTaskPrioritySet(greenBlinkTaskHandle, HIGH_PRIORITY);
            vTaskPrioritySet(NULL, LOW_PRIORITY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void greenBlinkTask(void *pvParameters) {
    for (;;) {
        if (uxTaskPriorityGet(NULL) == HIGH_PRIORITY) {
            stateGreenBlinking();
            // Because the greenblinking state already takes 4 seconds there is no need for an additional wait
            vTaskPrioritySet(yellowTaskHandle, HIGH_PRIORITY);
            vTaskPrioritySet(NULL, LOW_PRIORITY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void yellowTask(void *pvParameters) {
    for (;;) {
        if (uxTaskPriorityGet(NULL) == HIGH_PRIORITY) {
            stateYellow();
            vTaskDelay(pdMS_TO_TICKS(STATE_DURATION)); 
            vTaskPrioritySet(redTaskHandle, HIGH_PRIORITY);
            vTaskPrioritySet(NULL, LOW_PRIORITY);
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Main: Create tasks and start the scheduler

int main() {
    stdio_init_all();

    xTaskCreate(redTask, "Red", 1024, NULL, HIGH_PRIORITY, &redTaskHandle);
    xTaskCreate(redYellowTask, "RedYellow", 1024, NULL, LOW_PRIORITY, &redYellowTaskHandle);
    xTaskCreate(greenTask, "Green", 1024, NULL, LOW_PRIORITY, &greenTaskHandle);
    xTaskCreate(greenBlinkTask, "GreenBlink", 1024, NULL, LOW_PRIORITY, &greenBlinkTaskHandle);
    xTaskCreate(yellowTask, "Yellow", 1024, NULL, LOW_PRIORITY, &yellowTaskHandle);

    vTaskStartScheduler();

    for (;;);
    return 0;
}
