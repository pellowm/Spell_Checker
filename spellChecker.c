#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MATCH_COUNT 5

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char *key;
    while ((key = nextWord(file)) != NULL)
    {
        
        int index = 0;
        while (key[index] != 0)
        {
            key[index] = tolower(key[index]);
            index++;
        }
        hashMapPut(map, key, 0);
        free(key);
    }
}

/**
* Calculate three way integer minimum
* @param x first integer to compare
* @param y second integer to compare
* @param z third integer to compare
* @return smallest of the three input integers
*/
int minimum(int x, int y, int z)
{
    if ((x <= y) && (x <= z))
    {
        return x;
    }
    else if ((y <= x) && (y <= z))
    {
        return y;
    }
    else
    {
        return z;
    }
}

/**
 * Recursive Levenshtein Distance calculation code from:
 * https://en.wikipedia.org/wiki/Levenshtein_distance
 * @param s first input string
 * @param m number of characters in string s
 * @param t second input string
 * @param n number of characters in string t
 * @return the Levenshtein Distance between input strings s and t
 */
int LevenshteinDistance(const char *s, int m, const char *t, int n)
{
    /*calculate Levenshtein Distance*/
    int (*distance)[256] = malloc(sizeof(int) * 256 * 256);
 
    for (int i = 0; i <= m; i++)
    {
        for (int j = 0; j <= n; j++)
        {
            distance[i][j] = 0;
        }
    }

    for (int i = 1; i <= m; i++)
    {
        distance [i][0] = i;
    }

    for (int j = 1; j <= n; j++)
    {
        distance [0][j] = j;
    }

    int substitutionCost;
    for (int j = 1; j <= n; j++)
       {
           for (int i = 1; i <= m; i++)
           {
               if (s[i-1] == t[j-1])
               {
                   substitutionCost = 0;
               }
               else
               {
                   substitutionCost = 1;
               }
               distance[i][j] = minimum(distance[i-1][j] + 1,
                                        distance[i][j-1] + 1,
                                        distance[i-1][j-1] + substitutionCost);
           }
       }

    return  distance[m][n];
}

/**
* Checks input string for correctness and finds closets matches if needed
* @param inputWord the string input by user
* @param map dictionary hashmap
* @param matches blank array of five closest matching strings
* @return 1/0 or true/false for input string correctness
*/
int findMatches(char *inputWord, HashMap *map, char *matches[])
{
    /*check if the word is spelled correctly*/
    if (hashMapContainsKey(map, inputWord))
    {
        return 1;
    }

    /*if word spelled incorrectly, calculate distances for the dictionary*/
    HashLink *temp = NULL;
    for (int i = 0; i < map->capacity; i++)
    {
        temp = map->table[i];
        while (temp != NULL)
        {
            int distance = LevenshteinDistance(inputWord, strlen(inputWord),
                                temp->key, strlen(temp->key));
            
            hashMapPut(map, temp->key, distance);

            temp = temp->next;
        }
    }

    /*maintain sorted list of five smallest distances*/
    int minDistances[MATCH_COUNT] =
        {INT_MAX, INT_MAX, INT_MAX, INT_MAX, INT_MAX};
    for (int i = 0; i < map->capacity; i++)
    {
        temp = map->table[i];
        while (temp != NULL)
        {
            /*make a hole to add a new distance*/
            int j = MATCH_COUNT - 1;
            while (j >=0 && temp->value < minDistances[j])
            {
                if (j != MATCH_COUNT - 1)
                {
                    minDistances[j+1] =  minDistances[j];
                    matches[j+1] =  matches[j];
                }
                j--;
            }
            /*add the new distance*/
            if (j + 1 < MATCH_COUNT)
            {
                minDistances[j+1] = temp->value;
                matches[j+1] = temp->key;
            }
            
            temp = temp->next;
        }
    }

    return 0;
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);
    char *matches[5];

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    printf("size: %d\n", map->size);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);
        
        int index = 0;
        while (inputBuffer[index] != 0)
        {
            inputBuffer[index] = tolower(inputBuffer[index]);
            index++;
        }

        int result = findMatches(inputBuffer, map, matches);

        /*if word is correctly spelled*/
        if (result == 1)
        {
            printf("The inputted word %s is spelled correctly.\n", inputBuffer);
        }
        /*if word is incorrectly spelled*/
        else
        {
            printf("The inputted word %s is spelled incorrectly. ", inputBuffer);
            printf("Did you mean ");
            for (int i = 0; i < MATCH_COUNT; i++)
            {
                if (i <= MATCH_COUNT - 2)
                {
                    printf("%s, ", matches[i]);
                }
                else
                {
                    printf("or %s?\n", matches[i]);
                }
            }
        }

        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
    }

    hashMapDelete(map);
    return 0;
}
