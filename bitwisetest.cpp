/* Compile this with a C++ compiler Dev C++ or code blocks */
#include <stdio.h>
#include <conio.h>
#include <iostream.h>
#include <iostream>

using namespace std;

int main (int argc, const char * argv[]) {
    int noPermissions = 1;
    int hasPermissions = 2;
    int superPermissions = 4;
    
    int permissions = noPermissions | hasPermissions | superPermissions;

    int userPermissions = hasPermissions;
    int result = (userPermissions & permissions) ? printf("The user has permissions.") : printf("The user doesn't have any permissions.");
    cout<<result<<endl;
    getch();
    return 0;
}
