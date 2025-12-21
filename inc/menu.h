#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "style.h"

// Maximum number of sensor types configurable in the menu
#define MAX_SENSORS 9

// Maximum supported input image size
#define MAX_BMP_SIZE 1024
#define MAX_POINTS (MAX_BMP_SIZE * MAX_BMP_SIZE)

// Maximum characters in text input fields
#define MAX_INPUT 9
#define MAX_PARAMETERS_INPUT 12


// Sensor definition used by the menu
typedef struct {
    // coverage radius
    double range;        
    // cost per sensor
    double price;        
    // max available sensors of this type
    uint32_t quantity;   
} sensor_t;



/* --- Menu-controlled parameters --- */

// Sensor types defined in the UI
extern sensor_t* sensors;
extern uint8_t sensorsCount;

// PSO parameters controlled by the menu
extern uint32_t iterationCount;
extern uint32_t particlesCount;
extern double budget;



/* --- Menu API --- */

// Initialize menu state and apply UI style
void menuInit(style_t* s);

// Main menu loop
void menuHandle(void);

#endif // MENU_H
