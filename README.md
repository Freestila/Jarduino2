This is an extension / extended Version of the original Jarduino Code from Stilo (http://code.google.com/p/stilo/, published under GNU GPL V3). Since the original was in GPL V3, my code is also under GPL V3.

I've used the original code from Stilos Jarduino in Version V1.1 and changed it for my needs. The main changes are:

- My LEDs use a separate small ATMEL running Arduino code to dimm. To handle this i use the EasyTransfer Library to send the required values (only blue and white at the moment).
- I added code for a dosing pump with 4 pumps. They can be calibratet and you can have up to 4 dosage times per pump per day.
- I also changed parts of the wave pump code to allow switching pumps (a to b and back).


This is no complete Product. It works for my system (hardware is almost identical to the one from Stilo, except for dimming and dosing pump), but there are a couple of things i want to change.

Plans for the future:
- Change values remotely, either via bluetooth or webserver.
- Party-Mode with different light settings.

Since i have limited time, i can not tell when i will change things. I also plan to make a grafik library to make it easy to create and configure screens with buttons and so on, since this is realy ugly here.