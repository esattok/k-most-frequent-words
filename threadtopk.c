#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#define MAX_CHARS 26
#define MAX_WORD_SIZE 64

// Structures

struct file {
	char fileName[MAX_WORD_SIZE];
};

struct threadArg {
	int numOfWords;
	char filename[MAX_WORD_SIZE];
	struct wordFrequencyPair* heapPtr;
};

struct wordFrequencyPair {
    char word[MAX_WORD_SIZE];
    int frequency;
};

struct TrieNode
{
	int isCompleteWord;
	unsigned frequency;
	int minHeapId;
	struct TrieNode* child[MAX_CHARS];
};

struct MinHeapNode
{
	struct TrieNode* root;
	unsigned frequency;
	char* word;
};

struct MinHeap
{
	unsigned capacity; // Total capasity of the heap
	int count; // Number of filled slots in the heap
	struct MinHeapNode* array;
};


// Function Definitions
struct TrieNode* createTrieNode();
struct MinHeap* createMinHeap(int);
void swapMinHeapNodes(struct MinHeapNode*, struct MinHeapNode*);
void minHeapify(struct MinHeap*, int);
void populateMinHeap(struct MinHeap*);
void minHeapInsert(struct MinHeap*, struct TrieNode**, const char*);
void insertHeapAndTrie(struct TrieNode**, struct MinHeap*, const char*, const char*);
void insertHeapAndTrieWrapper(const char*, struct TrieNode**, struct MinHeap*);
void populateResultArray(struct MinHeap*, struct wordFrequencyPair*);
void getKMostFrequentWord(FILE*, int, struct wordFrequencyPair*);
static void* do_task(void*);


// Main Function

int main(int argc, char* argv[]) {
    int numOfWords = atoi(argv[1]);
    // If the number of words is 0 then no need to do any clculation and return
    if (numOfWords == 0) {
        return 0;
    }

	int numOfFiles = atoi(argv[3]);
	char outputFileName[MAX_WORD_SIZE];
	strcpy(outputFileName, argv[2]);
	struct file* fileTable = malloc(sizeof(struct file) * numOfFiles);

	pthread_t tids[numOfFiles];
	pthread_attr_t attr;
	struct threadArg tArgs[numOfFiles];
	struct wordFrequencyPair** pointerTable = malloc(sizeof(struct wordFrequencyPair*) * numOfFiles);
	
    for (int i = 0; i < numOfFiles; i++) {
		strcpy(fileTable[i].fileName, argv[i + 4]);
	}

	int ret;
    for (int i = 0; i < numOfFiles; i++) {
		// initialize thread argument to corresponding thread
		strcpy(tArgs[i].filename, fileTable[i].fileName);
		tArgs[i].numOfWords = numOfWords;
		pointerTable[i] = malloc(sizeof(struct wordFrequencyPair) * (numOfWords + 1));
		tArgs[i].heapPtr = pointerTable[i];

        pthread_attr_init(&attr);
		ret = pthread_create (&(tids[i]), &attr, do_task, (void *) &(tArgs[i]));

		if (ret != 0) {
			printf("thread create failed \n");
			exit(1);
		}
    }

    for (int i = 0; i < numOfFiles; i++) {
        ret = pthread_join(tids[i], NULL); 
		if (ret != 0) {
			printf("thread join failed \n");
			exit(1);
		}
    }
	
	// All threads terminated
	FILE* output_fp = fopen(outputFileName, "a");
    for (int i = 0; i < numOfFiles; i++) {
        for (int j = 0; j < pointerTable[i][0].frequency; j++) {
            for (int k = 0; k < pointerTable[i][j + 1].frequency; k++) {
                fprintf(output_fp, "%s\n", pointerTable[i][j + 1].word);
            }
        }
    }
    fclose(output_fp);

    output_fp = fopen(outputFileName, "r");
    struct wordFrequencyPair* finalListPtr = malloc(sizeof(struct wordFrequencyPair) * (numOfWords + 1));
    getKMostFrequentWord(output_fp, numOfWords, (struct wordFrequencyPair*)finalListPtr);
    fclose(output_fp);

    output_fp = fopen(outputFileName, "w");
    for (int i = 0; i < finalListPtr[0].frequency; i++) {
        fprintf(output_fp, "%s %d\n", finalListPtr[i + 1].word, finalListPtr[i + 1].frequency);
    }

	// Deallocation
	for (int i = 0; i < numOfFiles; i++) {
		free(pointerTable[i]);
	}

	free(pointerTable);
    free(finalListPtr);
    free(fileTable);
    fclose(output_fp);

    return 0;
}


// Function Definitions

// Function that the threads will execute
static void* do_task(void* arg_ptr) {
	FILE* fp_thread;
	char filename_thread[MAX_WORD_SIZE];

	strcpy(filename_thread, ((struct threadArg*)arg_ptr)->filename);
	fp_thread = fopen(filename_thread, "r");
	if (fp_thread == NULL) {
		printf("File does not exist");
	}

	getKMostFrequentWord(fp_thread, ((struct threadArg*)arg_ptr)->numOfWords, (struct wordFrequencyPair*)(((struct threadArg*)arg_ptr)->heapPtr));
	
	fclose(fp_thread);
	pthread_exit(NULL);
}

struct TrieNode* createTrieNode() {
	struct TrieNode* trieNode = malloc(sizeof(struct TrieNode));

	trieNode->isCompleteWord = 0;
	trieNode->frequency = 0;
	trieNode->minHeapId = -1;

	for (int i = 0; i < MAX_CHARS; i++) {
        trieNode->child[i] = NULL;
    }

	return trieNode;
}

struct MinHeap* createMinHeap(int capacity) {
	struct MinHeap* minHeap = malloc(sizeof(struct MinHeap));

	minHeap->capacity = capacity;
	minHeap->count = 0;
	minHeap->array = malloc(sizeof(struct MinHeapNode) * minHeap->capacity);

	return minHeap;
}

void swapMinHeapNodes(struct MinHeapNode* first, struct MinHeapNode* second) {
	struct MinHeapNode temp = *first;
	*first = *second;
	*second = temp;
}

void minHeapify(struct MinHeap* minHeap, int index) {
	int left, right, smallest;

	left = (2 * index) + 1;
	right = (2 * index) + 2;
	smallest = index;

	if ((left < minHeap->count) && (minHeap->array[left].frequency < minHeap->array[smallest].frequency)) {
        smallest = left;
    }
		
	if ((right < minHeap->count) && (minHeap->array[right].frequency < minHeap->array[smallest].frequency)) {
        smallest = right;
    }
		
	if (smallest != index) {
		minHeap->array[smallest].root->minHeapId = index;
		minHeap->array[index].root->minHeapId = smallest;

		swapMinHeapNodes (&(minHeap->array[smallest]), &(minHeap->array[index]));
		minHeapify(minHeap, smallest);
	}
}

void populateMinHeap(struct MinHeap* minHeap) {
	int countIndex = minHeap->count - 1;

	for (int i = (countIndex - 1) / 2; i >= 0; i--) {
        minHeapify(minHeap, i);
    }
}

void minHeapInsert(struct MinHeap* minHeap, struct TrieNode** root, const char* word) {
	if ((*root)->minHeapId != -1) {
		(minHeap->array[(*root)->minHeapId].frequency)++;
		minHeapify(minHeap, (*root)->minHeapId);
	}

	else if (minHeap->count < minHeap->capacity) {
		int count = minHeap->count;
		minHeap->array[count].frequency = (*root)->frequency;
		minHeap->array[count].word = malloc(sizeof(char) * (strlen( word ) + 1));
		strcpy(minHeap->array[count].word, word);

		minHeap->array[count].root = *root;
		(*root)->minHeapId = minHeap->count;
		(minHeap->count)++;

		populateMinHeap(minHeap);
	}

	else if ((*root)->frequency > minHeap->array[0].frequency) {
		minHeap->array[0].root->minHeapId = -1;
		minHeap->array[0].root = *root;
		minHeap->array[0].root->minHeapId = 0;
		minHeap->array[0].frequency = (*root)->frequency;

		// delete previously allocated memory and
		//delete [] minHeap->array[ 0 ]. word;
		minHeap->array[0].word = malloc(sizeof(char) * (strlen( word ) + 1));
		strcpy(minHeap->array[0].word, word);

		minHeapify(minHeap, 0);
	}
}

void insertHeapAndTrie(struct TrieNode** root, struct MinHeap* minHeap, const char* word, const char* duplicate) {
	// Base
	if (*root == NULL) {
	    *root = createTrieNode();
    }

	if (*word != '\0') {
        insertHeapAndTrie(&((*root)->child[tolower( *word ) - 97]), minHeap, word + 1, duplicate);
    }
		
	else {
		if ((*root)->isCompleteWord) {
            ((*root)->frequency)++;
        }
			
		else {
			(*root)->isCompleteWord = 1;
			(*root)->frequency = 1;
		}
		minHeapInsert( minHeap, root, duplicate );
	}
}


void insertHeapAndTrieWrapper(const char *word, struct TrieNode** root, struct MinHeap* minHeap) {
	insertHeapAndTrie(root, minHeap, word, word);
}

void populateResultArray(struct MinHeap* minHeap, struct wordFrequencyPair* temp_ptr) {
    // First element of the array is used to store the count of the frequent words since there may be less than k frequent words in the file
    temp_ptr[0].frequency = minHeap->count;
    strcpy(temp_ptr[0].word, "none_describes_count");

	for (int i = 0; i < minHeap->count; ++i) {
        temp_ptr[i + 1].frequency = minHeap->array[i].frequency;
        strcpy(temp_ptr[i + 1].word, minHeap->array[i].word);
	}
}

void getKMostFrequentWord(FILE* fp, int k, struct wordFrequencyPair* temp_ptr) {
	struct MinHeap* minHeap = createMinHeap(k);
	struct TrieNode* root = NULL;
	char buffer[MAX_WORD_SIZE];

	while (fscanf(fp, "%s", buffer) != EOF) {

        for (int i = 0; buffer[i]!='\0'; i++) {
            if(buffer[i] >= 'a' && buffer[i] <= 'z') {
                buffer[i] = buffer[i] - 32;
            }
        }

        insertHeapAndTrieWrapper(buffer, &root, minHeap);
    }

    // Heap Sort
    int count = minHeap->count;
    for (int i = count - 1; i >= 0; i--) {
        swapMinHeapNodes(&(minHeap->array[0]), &(minHeap->array[i]));
        (minHeap->count)--;
        minHeapify(minHeap, 0);
    }
    minHeap->count = count;

	populateResultArray(minHeap, temp_ptr);

    free(minHeap->array);
    free(minHeap);
}
