/**
 * Generator of the three adress code (language IFJcode17).
 * Uses list of syntax trees through which iterates and
 * from each node creates a code specific to
 * its type.
 * @author Tom Barbořík
 */
#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h> // output
#include <stdlib.h> // malloc / free
#include <string.h> // strcmp
#include "syntaxtree.h" // output

/**
 * Possible errors for the generator
 */
typedef enum {
    G_ERROR_NONE = 0,
    G_ERROR_MEMORY = 1,
    G_ERROR_STACK = 2,
    G_ERROR_NOT_EMPTY = 4
} g_error;

/**
 * Generates the final code used later for interpretation.
 * @param s_list
 * @param FILE *
 * @return int
 */
int generate(s_list, FILE *);

#endif