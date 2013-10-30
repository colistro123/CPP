/*
Documentation:
Classes: Some bugs left to fix, which I couldn't be bothered with. I just wanted to make an easy demonstration of what classes were but ended up creating a calculator.
*/

#include <stdio.h>
#include <cmath>
#include <math.h>
#include <conio.h>
#include <iostream>
#include <iostream.h>
using namespace std;

#define debug 1

#define DO_SUM 1
#define DO_DIVISION 2
#define DO_SQUAREROOT 3

#define MAX_OPTS 3 //Dev C++ is bugged, use CodeBlocks

char pEntClassName[3][36] = {
								{"Sum"},
								{"Division"},
								{"Square Root (Disabled)"}
							};
class CConstructor
{
public:
	//Constructor
	CConstructor() { cout<<"Instatiating Constructor()"<<endl;};
	//Destructor
	~CConstructor() { cout<<"Destroying Constructor()"<<endl; };
	int GetType() { return gType;}
	void SetType(int x) { gType = x; printf("Value is now %d \n", x); }
	void startMath( void );
	void onParseMath(int option);
	void showMethod(int option);
	int doCalculation( int fx, int fy, int option );
	void decideParse(int val1, int val2, int option);
	char* returnCalcType(int option);
	bool isValidOption(int option);
	bool is_integer(float k);
private: //Private variables so they can only be accessed within the class
	int gType;
	int total;
	float TotalDivision; //Divisions could give us floating values but it won't be used for now
};

int main(int argc, char *argv[]) {
    #if debug
    cout<<"int main called"<<endl;
    #endif
    CConstructor cPointer;
    cPointer.startMath();
}

void CConstructor::startMath() {
	int optinput = 0; //Initialize this as 0
		printf("Menu:\n");
    	for(int i=0; i<MAX_OPTS; i++) { //Loop through all those defined chars in the map and show them
		printf("%d. %s\n", i+1, pEntClassName[i]);	
	}
	printf("What would you like to do: ");
	cin>>optinput ?  onParseMath(optinput) : showMethod(optinput);
}

void CConstructor::onParseMath(int option) {
	if(!isValidOption(option)) {
		//Handle unhandled exceptions to prevent the program from closing unexpectedly?
		printf("Error, starting over..\n");
		startMath();                   
	} else {
		showMethod(option);
	} 
}

void CConstructor::showMethod(int option) {
	int val1, val2;
	printf("option: %d\n", option);
	printf("Enter your first value: ");
	cin>>val1;
	printf("Enter your second value: ");
	cin>>val2;
	decideParse(val1, val2, option);
}

void CConstructor::decideParse(int val1, int val2, int option) {
     if(!isValidOption(option)) {
        printf("Error, starting over..\n");
        startMath();
    }
	switch(option) {
		case DO_SUM: {
			doCalculation(val1, val2, option);
		}
	}
}
bool CConstructor::isValidOption(int option) {
	if(!is_integer(option)) {
		return false;
	}
	switch(option) {
		case DO_SUM: {
			return true;
		}
		default: {
			return false;
		}
	}
}
int CConstructor::doCalculation(int fx, int fy, int option) {
    if(!isValidOption(option)) {
		printf("Error, starting over..\n");
		startMath();
		return 0;
	}
    CConstructor cPointer;
	if(option == DO_SUM) {
		cPointer.total = fx + fy;
	} else {
		cPointer.total = fx / fy;
	}
    printf("The result from that %s is %d \n", returnCalcType(option), cPointer.total); 
    getch();
	startMath();
}
char* CConstructor::returnCalcType(int option) {
	char string[24];
	int index = option-1;
	#if debug
    	printf("Option: %d, Index: %d\n", option, index);
	#endif
	sprintf(string, "%s", pEntClassName[index]);
	return string; //We have to go backwards since our index will be 1 (sum would be 0, division would be 1, etc..
}
bool CConstructor::is_integer(float k) {
	return std::floor(k) == k;
}
