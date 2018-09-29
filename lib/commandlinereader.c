/*
// Command line reader, version 2
// Sistemas Operativos, DEI/IST/ULisboa 2018-19
*/

#include <string.h>
#include <stdio.h>

/**
Reads up to 'vectorSize' space-separated arguments from the standard input
and saves them in the entries of the 'argVector' argument.
This function returns once enough arguments are read or the end of the line
is reached

Arguments:
 'argVector' should be a vector of char* previously allocated with
 as many entries as 'vectorSize'
 'vectorSize' is the size of the above vector. A vector of size N allows up to
 N-1 arguments to be read; the entry after the last argument is set to NULL.
 'buffer' is a buffer with 'bufferSize' bytes, which will be
 used to hold the strings of each argument.

Return value:
 The number of arguments that were read, or -1 if some error occurred.
*/
int readLineArguments(char **argVector, int vectorSize, char *buffer, int bufferSize)
{
  int numTokens = 0;
  char *s = " \r\n\t";

  int i;

  char *token;

  if (argVector == NULL || buffer == NULL || vectorSize <= 0 || bufferSize <= 0)
     return 0;

  if (fgets(buffer, bufferSize, stdin) == NULL) {
    return -1;
  }

  /* get the first token */
  token = strtok(buffer, s);

  /* walk through other tokens */
  while( numTokens < vectorSize-1 && token != NULL ) {
    argVector[numTokens] = token;
    numTokens ++;

    token = strtok(NULL, s);
  }

  for (i = numTokens; i<vectorSize; i++) {
    argVector[i] = NULL;
  }

  return numTokens;
}

