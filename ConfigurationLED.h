/********
 * LED Brightness Configuration
 *******/

/* Minimum and maximum values for the LED brightness in relation to the ambient light.
 * LED_BRIGHT_MIN and LED_BRIGHT_MAX control the minimum and maximum brightness of the LED itself.
 * LED_LIGHT_MIN and LED_LIGHT_MAX control which ambient light level will count as minimum and which as maximum.
 */
#define LED_BRIGHT_MIN 8
#define LED_BRIGHT_MAX 255
#define LED_LIGHT_MIN  2
#define LED_LIGHT_MAX  15000

/* Enable the usage of a special color when the light reaches below the LED_LIGHT_MIN value. */
#define LED_LOW_ENABLE true

/* The color to use for the low light mode. */
RGB ledLowColor {0, 0, 0};


/********
 * Power LED Configuration
 *******/

/* Color of the power LED. */
RGB ledPowerColor {157, 217, 248};



/********
 * Notification LED Configuration
 *******/

/* Duration of a successful send pulse. */
#define LED_NOTIF_PULSE_SEND_DONE 50

/* Duration of a failed send pulse. */
#define LED_NOTIF_PULSE_SEND_FAIL 500

/* Duration of individual short sensor error pulses. */
#define LED_NOTIF_PULSE_SENSOR_ERROR 50

/* Notification colors. */
RGB ledNotifWiFiSearch {128, 128, 128};
RGB ledNotifWiFiFail   {192, 0, 0};

RGB ledSensorPMS_PM010 {157, 217, 248};
RGB ledSensorPMS_PM025 {157, 217, 248};
RGB ledSensorPMS_PM100 {157, 217, 248};

RGB ledSensorPMS_Pr003 {241, 62, 50};
RGB ledSensorPMS_Pr005 {246, 118, 52};
RGB ledSensorPMS_Pr010 {245, 235, 126};
RGB ledSensorPMS_Pr025 {122, 194, 64};
RGB ledSensorPMS_Pr050 {1, 147, 207};
RGB ledSensorPMS_Pr100 {102, 46, 138};

/* Enable color debugging.
 * The notification LED will keep glowing in the color set below. 
 * No sensor measurement will take place.
 */
#define LED_COLOR_DEBUG false
RGB ledDebug {32, 160, 64};
