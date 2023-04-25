#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_SIZE 32
#define BLOCK_SIZE 4

typedef struct cache {
    int size;   // Size of the cache
    int assoc;  // Cache's associativity
    int num_sets;   // Number of sets in the cache
    int block_size; // Size of a block in the cache
    int rp;     // Replacement policy (0 for LRU, 1 for Random)
    int hit;    // Number of cache hits
    int miss;   // Number of cache misses
    int **tag;  // Tag array
    int **valid;    // Valid array
    char **data;    // Data array
    int *lru;   // LRU array (only used for LRU replacement policy)
} Cache;

// Function to convert hexadecimal strings to integers
int hex_to_int(char *hex) {
    return (int) strtol(hex, NULL, 16);
}

// Function that initializes the cache
void init_cache(Cache *cache, int assoc, int rp)
{
    int i, j;
    cache->assoc = assoc;
    cache->num_sets = cache->size / (assoc * cache->block_size);
    cache->rp = rp;
    cache->hit = 0;
    cache->miss = 0;
    cache->tag = (int **) malloc(cache->num_sets * sizeof(int *));
    cache->valid = (int **) malloc(cache->num_sets * sizeof(int *));
    cache->data = (char **) malloc(cache->num_sets * sizeof(char *));


    if (rp == 0)
    {
        cache->lru = (int *) malloc(cache->num_sets * sizeof(int));
        memset(cache->lru, 0, cache->num_sets * sizeof(int));
    }

    // For loop that allocates memory to each array
    for (i = 0; i < cache->num_sets; i++)
    {
        cache->tag[i] = (int *) malloc(assoc * sizeof(int));
        cache->valid[i] = (int *) malloc(assoc * sizeof(int));
        cache->data[i] = (char *) malloc(cache->block_size * assoc * sizeof(char));


        for (j = 0; j < assoc; j++)
        {
            cache->tag[i][j] = -1;
            cache->valid[i][j] = 0;
            memset(&cache->data[i][j * cache->block_size], 0, cache->block_size * sizeof(char));
        }
    }
}

// Function to access the cache
void access_cache(Cache *cache, int addr)
{
    int set_index = (addr / cache->block_size) % cache->num_sets;
    int tag = (addr / cache->block_size) / cache->num_sets;
    int i, j, lru_index, random_index;


    for (i = 0; i < cache->assoc; i++)
    {

        if (cache->valid[set_index][i] && cache->tag[set_index][i] == tag)
        {

            cache->hit++;
            if (cache->rp == 0)
            {
                cache->lru[set_index] = i;
            }
            return;
        }
    }
        // Increments miss counter
        cache->miss++;

        if (cache->rp == 0)
        {
            lru_index = cache->lru[set_index];


            for (i = 0; i < cache->assoc; i++)
            {

                if (!cache->valid[set_index][i])
                {
                    lru_index = i;
                    break;
                }

                if (cache->lru[set_index] > cache->lru[set_index + i])
                {
                    lru_index = i;
                }
            }
            cache->lru[set_index] = cache->lru[set_index] + 1;

            // If statement that checks if index is max value and resets it and its corresponding block to 0
            if (lru_index == cache->assoc)
            {
                lru_index = 0;
                cache->lru[set_index] = 0;
            }

            cache->tag[set_index][lru_index] = tag;
            cache->valid[set_index][lru_index] = 1;
            memcpy(&cache->data[set_index][lru_index * cache->block_size], &addr, cache->block_size * sizeof(char));
        }
        
        // else statement that picks random index within the set and updates tag and data for that index
        else
        {
            random_index = rand() % cache->assoc;
            cache->tag[set_index][random_index] = tag;
            cache->valid[set_index][random_index] = 1;
            memcpy(&cache->data[set_index][random_index * cache->block_size], &addr, cache->block_size * sizeof(char));
        }
}


int main()
{
    Cache cache;
    char hexLength[9];
    int address;

    // Open the file for reading
    FILE *file = fopen("traces.txt", "r");
    

    // Checks if the file is populated
    if(file == NULL)
    {
        printf("There was an ERROR opening the file!");
        return 1;
    }
    
    // Initialize cache with size 32, associativity 1, block size 4, and replacement policy LRU
    cache.size = CACHE_SIZE;
    cache.block_size = BLOCK_SIZE;
    init_cache(&cache, 1, 0);

    // While loop that continues until the end of the text file
    while(fgets(hexLength, sizeof(hexLength), file) != NULL)
    {
        fscanf(file, "%s", hexLength);

        // Convert hexadecimal address to integer
        address = hex_to_int(hexLength);

        // Access the cache using the address
        access_cache(&cache, address);
    }
    
    // Prints out the cache hit/miss
    printf("\nHits: %d\n", cache.hit);
    printf("Misses: %d\n\n", cache.miss);

    // Close the file
    fclose(file);
    return 0;
}