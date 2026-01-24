/*
	--------------------------------------------------

	# Information

	File: pwc_logger.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides a convenient logger for the project.

	--------------------------------------------------

	## References
	
	1.https://www.geeksforgeeks.org/c/enumeration-enum-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with enums in C.

	2. https://stackoverflow.com/questions/3134757/define-or-enum
	-> I used this article to theorize between using an enum or defines for my log levels.

	3. https://stackoverflow.com/questions/1442116/how-can-i-get-the-date-and-time-values-in-a-c-program
	4. https://stackoverflow.com/questions/9101590/fprintf-and-ctime-without-passing-n-from-ctime
	5. https://www.geeksforgeeks.org/cpp/strftime-function-in-c/
	6. https://man7.org/linux/man-pages/man3/strftime.3.html
	-> I used these resources to debate the best way to format my datetime string.

	7. https://www.geeksforgeeks.org/c/c-switch-statement/
	-> I used this GeeksForGeeks article to refamiliarize myself with switch syntax.

	--------------------------------------------------
*/

// Log File Path
#define PWC_LOGFILE_PATH "pwc_logfile.log"

// Logger Level Enum
enum pwc_loggerLevel {
	PWC_LOGLEVEL_INFO = 0,
	PWC_LOGLEVEL_DEBUG = 1,
	PWC_LOGLEVEL_WARNING = 2,
	PWC_LOGLEVEL_ERROR = 3
};


/*
	# pwc_logToFile

	This function will log a message to the configured log file.

	## Parameters
	- enum pwc_loggerLevel level: The log level of the message.
	- const char* module: The module the message originated.
	- const char* message: The message to log.

	## Returns
	- void
*/
void pwc_logToFile(enum pwc_loggerLevel level, const char* module, const char* message);