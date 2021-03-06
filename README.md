# Dashie
Dashie is an ESP8266-powered particle detection sensor designed to analyze and send environmental measurements to [VariPass](https://varipass.org/). The sensor used internally is the Plantower PMS5003 sensor, and it reports the following values:
- Particles >0.3 um / 0.1L
- Particles >0.5 um / 0.1L
- Particles >1.0 um / 0.1L
- Particles >2.5 um / 0.1L
- Particles >5.0 um / 0.1L
- Particles >10.0 um / 0.1L

Dashie was primarily designed to look good AND serve the functionality. Inspired by *My Little Pony: Friendship is Magic*'s character Rainbow Dash, the device uses an upper RGB LED to signify the power state, glowing solidly in the color of Dash's coat. The lower RGB LED signifies whether the device is sending data to VariPass. On successful sends, the LED flashes in the colors Dash's rainbow mane. Both LEDs change their intensity depending on the ambient lighting which is retrieved from VariPass, as measured by the [Celly](https://github.com/Thorinair/Celly) sensor system in order to not be too obtrusive during night time.