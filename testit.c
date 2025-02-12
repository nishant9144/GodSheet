#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LEN 3

// Function to convert column name to number
int colNameToNumber(const char *colName) {
    int result = 0;
    while (*colName) {
        if (!isalpha(*colName)) return -1; // Invalid character check
        result = result * 26 + (toupper(*colName) - 'A' + 1);
        colName++;
    }
    result--; // 0 based indexing
    return result;
}

void colNumberToName(int colNumber, char *colName) { // 0 based argument
    if (colNumber < 0 || colNumber > 18277) { // Limit for "ZZZ"
        strcpy(colName, "\0");
        return;
    }
    colNumber++;
    int index = 0;
    char temp[4];
    while (colNumber > 0) {
        colNumber--;
        temp[index++] = 'A' + (colNumber % 26);
        colNumber /= 26;
    }
    temp[index] = '\0';
    
    // Reverse the result to get correct column name
    int len = strlen(temp);
    for (int i = 0; i < len; i++) {
        colName[i] = temp[len - 1 - i];
    }
    colName[len] = '\0';
    return;
}

int main() {
    char *colName = (char *)malloc(4 * sizeof(char));
    int colNumber;
    
    // Test column name to number
    printf("Column Name to Number:\n");
    printf("A -> %d\n", colNameToNumber("A"));
    printf("Z -> %d\n", colNameToNumber("Z"));
    printf("AA -> %d\n", colNameToNumber("AA"));
    printf("ZZZ -> %d\n", colNameToNumber("ZZZ"));
    
    // Test column number to name
    printf("\nColumn Number to Name:\n");
    colNumberToName(0, colName);
    printf("0 -> %s\n", colName);
    colNumberToName(30, colName);
    printf("26 -> %s\n", colName);
    colNumberToName(18277, colName);
    printf("18277 -> %s\n", colName);
    colNumberToName(18278, colName);
    printf("18278 -> %s\n", colName);
    
    return 0;
}
