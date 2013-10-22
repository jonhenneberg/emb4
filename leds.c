#include <iostream>
#include <stdio.h>
#include <unistd.h>
using namespace std;

int main(){
cout << "LED Flash Start" << endl;
FILE *LEDHandle = NULL;
const char *LEDBrightness="/sys/class/leds/beaglebone:green:usr0/brightness";

for(int i=0; i<100; i++){ 
	if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL){
		fwrite("1", sizeof(char), 1, LEDHandle);
		fclose(LEDHandle);
	}
	usleep(20000);

	if((LEDHandle = fopen(LEDBrightness, "r+")) != NULL){
		fwrite("0", sizeof(char), 1, LEDHandle);
		fclose(LEDHandle);
	}
	usleep(20000);
 }
 cout << "LED Flash End" << endl;
}
