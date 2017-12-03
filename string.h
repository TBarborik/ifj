/**
 * String library. It takes care of char array to make sure
 * there is enough space to store neccessary content.
 * It is made as linear list with one node space of
 * STR_MAXLEN chars. Overall capacity is unsigned
 * long size. Every string ends with \0. 
 * To achieve it, every char array
 * to store must end with \0
 * as well.
 * 
 * @author Tom Barbořík
 */
#ifndef STRING_H
#define STRING_H

#include <stdio.h> // outputing
#include <stdlib.h> // memory
#include <string.h> // comparing

#define STR_MAXLEN 256

typedef struct str {
    char *arr; // array with the string
    unsigned int len; // length of the string including zero char
    struct str *next; // if the string is too long, next will be created
} *string;

/**
 * Creates new instance of string
 * @return string
 */
string str_new();

/**
 * Removes strings and takes care of memory
 */
void str_del(string s);

/**
 * Appends char *arr at the end of given string
 * @param s string
 * @param arr *char
 */
void str_append(string s, char *arr);

/**
 * Appends a char at the end of given string
 * @param s string
 * @param c char
 */  
void str_append_char(string s, char c);

/**
 * Removes content of the string and puts there a new one
 * from char *arr
 * @param s string
 * @param arr *char
 */
void str_put(string s, char *arr);

/**
 * Removes content of the string and puts there
 * a char
 * @param s string
 * @param c char
 */
void str_put_char(string s, char c);

/**
 * Return length of the string including \0
 * @param s string
 * @return unsigned long
 */ 
unsigned long str_len(string s);

/**
 * Clears string
 * @param s string
 */
void str_clear(string s);

/**
 * Contacts two strings together into one.
 * s1 = s1 + s2
 * @param s1 string
 * @param s2 string
 */
void str_concat(string s1, string s2);

/**
 * Prints content of the string into file
 * @param s string
 * @param file *FILE
 */
void str_println(string s, FILE *f);

/**
 * Creates char array with content from string.
 * Be aware of memory leaks. It neccessary to
 * free memory of the array.
 * @param s string
 * @return char*
 */
char *str_to_array(string s);

/**
 * Compares two strings.
 * @param s1 string
 * @param s2 string
 * @return int
 */
int str_cmp(string s1, string s2);

#endif
