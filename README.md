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

Development
-----------

The Roboto font was used to draw all the digit images.

Watchface may easily be made to vibrate the watch on the hour, just modify the "(src/big-h.c)" to set the "vibrate_on_hour" variable to "true" in the "SETTINGS" section of the code as follows:

    /************** SETTINGS - START **************/

    // set to true if you'd like your pebble to vibrate on the hour
    // ORIGINAL CODE: vibrate_on_hour = false;
    vibrate_on_hour = true;

    [...]

  	/************** SETTINGS - END **************/


Watchface may also be modified to disregard the 12/24-hour setting when choosing the date format to use by modifying the same "(src/big-h.c)" file to set the "us_date_format" variable in the "SETTINGS" section to "true" to use MM-DD-YYYY or to "false" to use YYYY-MM-DD.  See below:

	/************** SETTINGS - START **************/

	[...]
	  
	// by default, the date will be YYYY-MM-DD if the clock is 24-hour style and MM-DD-YYYY if the clock is 12-hour style
	// change to true if you'd like to always use US-style date format MM-DD-YYYY
	// change to false if you'd like to always use the YYYY-MM-DD format
	// ORIGINAL CODE: us_date_format = !clock_is_24h_style();
	us_date_format = true;

	/************** SETTINGS - END **************/


License
-------

Licensed under the Lesser GNU Public License version 3 (LGPLv3)
see included LICENSE file or http://www.gnu.org/copyleft/lesser.html