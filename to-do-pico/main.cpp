#include "pico/stdlib.h"
#include <stdio.h>

#define MAX 2

char t1[15] = "";
char t2[15] = "";
bool d1 = false;
bool d2 = false;
int cnt = 0;

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\nREADY\n");

    while (1) {
        printf("\n1=List 2=Add 3=Done 4=Del\n>");
        
        char c = getchar();
        getchar(); // consume newline

        if (c == '1') {
            // List
            printf("\nTasks:\n");
            if (cnt >= 1) printf("1. [%c] %s\n", d1?'X':' ', t1);
            if (cnt >= 2) printf("2. [%c] %s\n", d2?'X':' ', t2);
            if (cnt == 0) printf("None\n");
            
        } else if (c == '2') {
            // Add
            if (cnt == 0) {
                printf("Task: ");
                scanf("%14s", t1);
                getchar();
                d1 = false;
                cnt = 1;
                printf("OK\n");
            } else if (cnt == 1) {
                printf("Task: ");
                scanf("%14s", t2);
                getchar();
                d2 = false;
                cnt = 2;
                printf("OK\n");
            } else {
                printf("FULL\n");
            }
            
        } else if (c == '3') {
            // Done
            printf("Which? ");
            char n = getchar();
            getchar();
            if (n == '1' && cnt >= 1) {
                d1 = true;
                printf("OK\n");
            } else if (n == '2' && cnt >= 2) {
                d2 = true;
                printf("OK\n");
            }
            
        } else if (c == '4') {
            // Delete
            printf("Which? ");
            char n = getchar();
            getchar();
            if (n == '1' && cnt >= 1) {
                if (cnt == 2) {
                    // Move task 2 to task 1
                    for (int i = 0; i < 15; i++) t1[i] = t2[i];
                    d1 = d2;
                }
                cnt--;
                printf("OK\n");
            } else if (n == '2' && cnt == 2) {
                cnt--;
                printf("OK\n");
            }
        }
    }
}
