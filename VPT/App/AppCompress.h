#pragma once
#include "../processing.h"

class CAppCompress :
	public CProcessing
{
public:
	// Add variables here

public:
	CAppCompress(void);
	~CAppCompress(void);
	// Add methods here

	unsigned char *Compress(int &cDataSize) ;
	void Decompress(unsigned char *compressedData, int cDataSize, unsigned char *deCompressedData) ;

public:
	void CustomInit(CView *pView) ;
	void Process(void) ;
	void CustomFinal(void) ;
};

/**
 * Huffman Tree Node
 */
struct treeNode {
	int key;
	int value;
	treeNode* parent;
	treeNode* leftChild;
	treeNode* rightChild;
};

void select(treeNode**, int*, int*, int);
void buildHuffmanTree(treeNode**, int);
int DictBuild(unsigned char*, int, unsigned char*, int*);
treeNode* treeBuild(unsigned char*, int*);

/**
 * Calculate the length that complement binary bit
 * examples:
 *       5 ->  8
 *       9 -> 16
 *      17 -> 32
 */
int calcComplementLength(int);

/* Automatically adjust size of unsigned char* */
void writeData(unsigned char* &target, int& size, int pos, unsigned char data);
