#include <iostream>
#include <pthread.h>
#include "huffmanTree.h"
using namespace std;

// create a struct to hold the Huffman tree and compressed data
struct ThreadArgs
{
	HuffmanTree *tree;
	const string *binaryCode;
	vector<int> *position;
	pthread_mutex_t *mutex;
	char *buffer;
};
// You can only use named POSIX semaphores, pthreads mutex semaphores, or pthreads condition variables to achieve synchronization.
void *decompressSymbol(void *arg)
{
	ThreadArgs *threadArg = (ThreadArgs *)arg;
	HuffmanTree *tree = threadArg->tree;
	const string *code = threadArg->binaryCode;
	vector<int> *position = threadArg->position;
	pthread_mutex_t *mutex = threadArg->mutex;

	// write the decompressed data to the buffer
	pthread_mutex_lock(mutex);
	for (int i = 0; i < position->size(); i++)
	{
		threadArg->buffer[position->at(i)] = tree->decode(*code);
	}
	pthread_mutex_unlock(mutex);

	return nullptr;
}

int main(int argc, char *argv[])
{
	HuffmanTree huffmanTree;
	vector<string> printChar;
	int lineCount;
	int max_pos = 0;
	map<string, vector<int>> charIndex;
	// read the frequency from stdin
	huffmanTree.readFreqFromInput(cin);
	huffmanTree.createTree();
	lineCount = huffmanTree.getAlphabetSize();

	// continue to read the compressed info from stdin
	for (int i = 0; i < lineCount; i++)
	{
		string line;
		getline(cin, line);
		istringstream iss(line);
		string code;
		int num;
		// compressed code
		iss >> code;
		while (iss >> num)
		{
			charIndex[code].push_back(num);
			// record the max position index
			if (num > max_pos)
			{
				max_pos = num;
			}
		}
		printChar.push_back(code);
	}

	// Create n POSIX threads (where n is the number of symbols in the alphabet).
	// Each child thread executes the Receives the Huffman tree and the information about the symbol to decompress (binary code and list of positions) from the main thread.
	pthread_t threads[lineCount];
	pthread_mutex_t mutex;
	auto it = charIndex.begin();
	char buffer[1024];
	pthread_mutex_init(&mutex, nullptr);
	for (int i = 0; i < lineCount; i++)
	{
		auto *args = new ThreadArgs;
		args->tree = &huffmanTree;
		args->binaryCode = &it->first;
		args->position = &it->second;
		args->mutex = &mutex;
		args->buffer = buffer;
		pthread_create(&threads[i], nullptr, decompressSymbol, (void *)args);
		it++;
	}
	// Wait for all threads to complete
	for (int i = 0; i < lineCount; i++)
	{
		pthread_join(threads[i], nullptr);
	}
	pthread_mutex_destroy(&mutex);

	// print the huffman code
	huffmanTree.printCode(printChar);
	// get the decompressed data from buffer
	buffer[max_pos + 1] = '\0';
	// print the decompressed data
	cout << "Original message: " << buffer << endl;
	return 0;
}