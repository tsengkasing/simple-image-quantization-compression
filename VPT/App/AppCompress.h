#pragma once
#include "../processing.h"

struct treeNode {
	int key;
	int value;
	treeNode* parent;
	treeNode* leftChild;
	treeNode* rightChild;
};

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
	int DictBuild(unsigned char* pInput, int cDataSize, char* dict, int* lens);
	void InsertNode(treeNode** node, treeNode* newNode);
	void buildHuffmanTree(treeNode** node, int n);
};


