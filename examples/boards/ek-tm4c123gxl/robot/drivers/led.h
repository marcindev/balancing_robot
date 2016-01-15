#ifndef LED_H
#define LED_H
#include <stdbool.h>
#include <stdint.h>


typedef enum {LED1 = 0, LED2, LED3, LED4, LED5, LED6,LED7, LED8, LED9, LED10, LED11} Led;

void LedInit(Led led);
void LedTurnOn(Led led);
void LedTurnOff(Led led);
void LedTurnOnOff(Led led, bool isOn);
void LedBlinkStart(Led led, uint32_t periodSec);
void LedBlinkStop(Led led);


#endif // LED_H
