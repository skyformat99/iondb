/* automatically generated by arduino-cmake */
#line 1 "/Users/danaklamut/ClionProjects/iondb/src/tests/unit/dictionary/flat_file/flat_file.ino"
#include <SPI.h>
#include <SD.h>
#include "test_flat_file.h"
#include "test_flat_file_dictionary_handler.h"

#line 8 "/Users/danaklamut/ClionProjects/iondb/cmake-build-debug/src/tests/unit/dictionary/flat_file/test_flat_file_flat_file.ino.cpp"
#include "Arduino.h"

/* === START Forward: /Users/danaklamut/ClionProjects/iondb/src/tests/unit/dictionary/flat_file/flat_file.ino */
void
setup(
);

void
setup(
);

void
loop(
);

void
loop(
);

/* === END Forward: /Users/danaklamut/ClionProjects/iondb/src/tests/unit/dictionary/flat_file/flat_file.ino */
#line 4 "/Users/danaklamut/ClionProjects/iondb/src/tests/unit/dictionary/flat_file/flat_file.ino"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_flat_file();
	runalltests_flat_file_handler();
}

void
loop(
) {}