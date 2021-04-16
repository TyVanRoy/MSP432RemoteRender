#include "RedLib.h"

/*
 * Debug and Printing
 */

inline void TimeDebug(char* tag, int i){
    char str[255];
    snprintf((str), 255, "%d ms\t*DEBUG*\t\t%s\t%d\n\r", SystemTime, tag, i);
    Print(str);
}

inline void Debug(char* tag, int i){
    char str[255];
    snprintf((str), 255, "*DEBUG*\t\t%s:\t%d\n\r", tag, i);
    Print(str);
}

inline void Print(char* string){
    BackChannelPrint(string, none);
}

inline void PrintLn(char* string){
    char str[255];
    snprintf((str), 255, "%s\n\r", string);
    Print(str);
}

inline void PrintInt(int i){
    char str[255];
    snprintf((str), 255, "%d", i);
    Print(str);
}

inline void PrintFloat(float f){
    char str[255];
    snprintf((str), 255, "%f", f);
    Print(str);
}

/*
 * Utilities
 */

int NewtRoot(int x){

    int xk;
    int xk1 = x;
    int n = x;
    int absDif;
    do{
        xk = xk1;
        xk1 = (xk + (n / xk)) >> 1;

        absDif = xk1 - xk;
        absDif = absDif < 0 ? -absDif : absDif;
    }while(absDif >= 1);

    return xk1;
}
