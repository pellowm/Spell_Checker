/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Megan Pellow
 * Date: 11/26/2019
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link pointer to link structure
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map pointer to the HashMap structure
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map pointer to the HashMap structure
 */
void hashMapCleanUp(HashMap* map)
{
    /*delete all links and free memory allocated for table*/
    HashLink *cur;
    HashLink *temp;
    for (int i = 0; i < map->capacity; i++)
    {
        if (map->table[i] != NULL)
        {
            cur = map->table[i];
            while (cur != NULL)
            {
                temp = cur->next;
                hashLinkDelete(cur);
                cur = temp;
            }
        }
    }
    free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map pointer to the HashMap structure
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map pointer to the HashMap structure
 * @param key string used as hash map key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    /*find address of value associated with key*/
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
    if (hashIndex < 0)
    {
        hashIndex += map->size;
    }

    HashLink *temp = map->table[hashIndex];
    while (temp != NULL)
    {
        if (strcmp(temp->key, key) == 0)
        {
            return &(temp->value);
        }
       temp = temp->next;
    }
    /*if value not found, return NULL*/
    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map pointer to the HashMap structure
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    int oldCap = hashMapCapacity(map);
    HashLink **oldTable = map->table;

    /*initalize a new hashmap structure*/
    hashMapInit(map, capacity);

    /*move current hash map contents into new larger hash map*/
    for (int i = 0; i < oldCap; i++)
    {
        HashLink *cur = oldTable[i];
        while (cur != NULL)
        {
            int hashIndex = HASH_FUNCTION(cur->key) % map->capacity;
            
            HashLink *temp = cur->next;
            cur->next = map->table[hashIndex];
            map->table[hashIndex] = cur;
            cur = temp;
            map->size++;
        }
    }
    free(oldTable);
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map pointer to the HashMap structure
 * @param key string used as hash map key
 * @param value integer used as hash map value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    /*if key already exists in hash map, update value*/
    int *valPtr = hashMapGet(map, key);
    if (valPtr != NULL)
    {
        *valPtr = value;
    }
    /*if key is not yet in hash map, create new link*/
    else
    {
        int hashIndex = HASH_FUNCTION(key) % map->capacity;
        HashLink *next = map->table[hashIndex];
        map->table[hashIndex] = hashLinkNew(key, value, next);
        map->size++;

        /*check if table needs to be resized*/
        if (hashMapTableLoad(map) >= MAX_TABLE_LOAD)
        {
            resizeTable(map, 2*map->capacity);
        }
    }
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map pointer to the HashMap structure
 * @param key string used as hash map key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    /*search for key in hashmap to delete*/
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
   
    if (hashIndex < 0)
    {
        hashIndex += map->size;
    }
    /*case 1: if key is not in hash map*/
    HashLink *cur = map->table[hashIndex];
    if (cur == NULL)
    {
        return;
    }
    /*case 2: if key is first link in bucket*/
    else if (strcmp(cur->key, key) == 0)
    {
        map->table[hashIndex] = cur->next;
        hashLinkDelete(cur);
        map->size--;
        return;
    }
    /*case 3: if key is any other link in bucket*/
    else
    {
        HashLink *prev = cur;
        cur = cur->next;
        while (cur != NULL)
        {
            if (strcmp(cur->key, key) == 0)
            {
                prev->next = cur->next;
                hashLinkDelete(cur);
                map->size--;
                return;
            }
            prev = prev->next;
            cur = cur->next;
        }
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map pointer to the HashMap structure
 * @param key string used as hash map key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    /*search hash map for key*/
    int hashIndex = HASH_FUNCTION(key) % map->capacity;
    if (hashIndex < 0)
    {
        hashIndex += map->size;
    }

    HashLink *temp = map->table[hashIndex];
    while (temp != NULL)
    {
        if (strcmp(temp->key, key) == 0)
        {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map pointer to the HashMap structure
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map pointer to the HashMap structure
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map pointer to the HashMap structure
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    int emptyBuckets = 0;

    for (int i = 0; i < map->capacity; i++)
    {
        if (map->table[i] == NULL)
        {
            emptyBuckets++;
        }
    }
    return emptyBuckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map pointer to the HashMap structure
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    return (float)map->size / (float)map->capacity;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map pointer to the HashMap structure
 */
void hashMapPrint(HashMap* map)
{
    HashLink *temp = NULL;
    for (int i = 0; i < map->capacity; i++)
    {
        temp = map->table[i];
        while (temp != NULL)
        {
            printf("%d, ", temp->value);
            temp = temp->next;
        }
    }
}
