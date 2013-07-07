Big H
=====

Big H, watchface for Pebble smartwatch

![Animated rendering of the Big H Pebble watchface](pebble-watchface_big-h.gif)

Description
-----------

Big H is a watchface that shows the hour and minutes in big numbers, the weekday with the dates of 3 days before and after, the full date and a seconds indicator with minimal battery impact.

Features
--------

* Hours in big numbers in the top center (12-hour & 24-hour formats)
* Minutes in big numbers in the bottom center
* Weekday indicator column on the left side of the screen
	* 7 numbers shown; top slot is for Sunday, last slot is Saturday
	* Current date is in reverse color (black on white) in the appropriate slot for its day
	* 3 days before and after the current day are shown
	* Arrow drawing indicates the break between the past 3 days and the future 3 days
* Full date is written in the rightmost column
	* If watch is set to 12-hour format, the date will show as MM-DD-YYYY
	* Otherwise, the date will be formatted as YYYY-MM-DD
* Seconds indicator is in the center, between the hour and minutes
	* Empty bar is 0 seconds
	* Full bar is 59 seconds
	* Indicators are present to indicate 15s, 30s and 45s
	* Seconds indicator only uses 2 lines so very battery efficient but still visible
* If the watch is in 12-hour format:
	* AM is indicated by a small arrow pointing up over the center of the seconds bar
	* PM is indicated by the small arrow pointing down under the center of the seconds bar

Customizations
--------------

Changing settings and recompiling may make the following changes:

* Watch can be made to vibrate on the hour
* Hours can be shown with the leading zero
* Weekday indicator can be replaced with spelled-out weekdays in:
        * English
        * French
        * Spanish
* Weekday indicator can be made so the first position is Monday rather than Sunday
* Seconds indicator can be removed so the watchface only updates once per minute (even more battery efficient)
* Date can be formatted in probably any format


Development
-----------

The Roboto font was used to draw all the characters/numbers.

To modify the settings listed under "Customizations":
1. Open the "src/big-h.c" file
2. Find the "SETTINGS" section at the beginning of the file (not the very top, but not too far down)
3. Modify the settings as you wish (the comments should explain what each does and how to change it)
4. Recompile!

License
-------

Licensed under the Lesser GNU Public License version 3 (LGPLv3)
see included LICENSE file or http://www.gnu.org/copyleft/lesser.html
