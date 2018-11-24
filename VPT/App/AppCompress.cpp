#include "StdAfx.h"
#include "AppCompress.h"

CAppCompress::CAppCompress(void)
{
	// Class Constructor
}

CAppCompress::~CAppCompress(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppCompress::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
}

void CAppCompress::CustomFinal(void) {
	// Add custom finalization code here
}


int predictEval(unsigned char *buf, int x, int y, int width, int height, int &diffValue) {

	int predT ;
	int predL ;
	int predTL ;
	int pred ;
	int left ;
	int top ;
	int topLeft ;
	int mode ;
	int actual ;
	int diff ;

	if(x <= 0) {
		left = 0 ;
		topLeft = 0 ;
	}
	if(y <= 0) {
		top = 0 ;
		topLeft = 0 ;
	}
	if(y > 0) {
		top = buf[x + (y - 1) * width] ;
	}
	if(x > 0 && y >= 0) {
		left = buf[(x - 1) + y * width] ;
	}
	if(x > 0 && y > 0) {
		topLeft = buf[(x - 1) + (y - 1) * width] ;
	}

	predT = top ;
	predL = left ;
	predTL = topLeft ;
	actual = buf[x + y * width] ;

	if(predL <= predT && predL <= predTL) {
		mode = 1 ;
		pred = predL ;
	} else if(predT <= predL && predT <= predTL) {
		mode = 2 ;
		pred = predT ;
	} else if(predTL <= predL && predTL <= predT) {
		mode = 3 ;
		pred = predTL ;
	}
	diff = actual - pred ;
	diff = diff >= 0 ? diff : -diff ;
	if(diff >= 8) {
		mode = 4 ;
		diffValue = actual ;
	} else {
		diffValue = actual - pred ;
	}
	return mode ;
}

unsigned char predDiff(unsigned char *buf, int x, int y, int width, int height, int mode, int diffValue) {

	int predT ;
	int predL ;
	int predTL ;
	int pred ;
	int left ;
	int top ;
	int topLeft ;

	if(x <= 0) {
		left = 0 ;
		topLeft = 0 ;
	}
	if(y <= 0) {
		top = 0 ;
		topLeft = 0 ;
	}
	if(y > 0) {
		top = buf[x + (y - 1) * width] ;
	}
	if(x > 0 && y >= 0) {
		left = buf[(x - 1) + y * width] ;
	}
	if(x > 0 && y > 0) {
		topLeft = buf[(x - 1) + (y - 1) * width] ;
	}
	predT = top ;
	predL = left ;
	predTL = topLeft ;

	switch(mode) {
	case 1:
		pred = predL + diffValue;
		break ;
	case 2:
		pred = predT + diffValue;
		break ;
	case 3:
		pred = predTL + diffValue;
		break ;
	case 4:
		pred = diffValue ;
		break ;
	}

	return (unsigned char) pred ;
}

// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {

	// You can modify anything within this function, but you cannot change the function prototype.
	unsigned char *compressedData ;

	TRACE("width: %d, height: %d\r\n", width, height);
	cDataSize = width * height * 3 ;				// You need to determine the size of the compressed data. 
													// Here, we simply set it to the size of the original image

	compressedData = new unsigned char[cDataSize] ; // As an example, we just copy the original data as compressedData.

	memcpy(compressedData, pInput, cDataSize) ;

	TRACE("cDataSize: %d\r\n", cDataSize);

	
	char dict[256*32];
	for (int i = 0; i < 256*32; i++) {
			dict[i] = '\0';
	}
	int* lens = new int[256];
	int max_len = DictBuild(pInput, cDataSize, dict, lens);

	char temp[32];
	memcpy(temp, dict, 32);
	for (int i = 0; i < 32 * 8; i++) {
		TRACE("####%c\r\n", ((temp[i / 8] >> (i % 8)) & ('1' - '0')) + '0');
	}
	TRACE("****%d\r\n", lens[0]);

	TRACE("max_len: %d\r\n", max_len);

	return compressedData ;		// return the compressed data
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

	// You can modify anything within this function, but you cannot change the function prototype.
	memcpy(uncompressedData, compressedData, cDataSize) ;	// Here, we simply copy the compressedData into the output buffer.
}


void CAppCompress::Process(void) {

	// Don't change anything within this function.

	int i, cDataSize ;

	unsigned char *compressedData ;
	unsigned char *verifyCompressedData ;

	SetTitle(pOutput, _T("Lossless Decompressed Image")) ;

	compressedData = Compress(cDataSize) ;

	verifyCompressedData = new unsigned char [cDataSize] ;

	memcpy(verifyCompressedData, compressedData, cDataSize) ;

	delete [] compressedData ;

	Decompress(verifyCompressedData, cDataSize, pOutput) ;

	for(i = 0; i < width * height * 3; i++) {
		if(pInput[i] != pOutput[i]) {
			printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n")) ;
			break ;
		}
	}

	printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize, (double) width * height * 3 / cDataSize) ;

	PutDC(pOutput) ;
}


void CAppCompress::InsertNode(treeNode** node, treeNode* newNode) {
	for (int i = 2; i < 256; i++) {
		if (newNode->value < node[i]->value || node[i]->value==0) {
			for (int j = 2; j < i; j++) {
				node[j - 2] = node[j];
			}
			node[i - 2] = newNode;
			for (int j = i - 1; j < 256; j++) {
				node[j] = node[j + 1];
			}
		}
	}
}

void select(treeNode** node, int* i1, int* i2, int n) {
	for (int i = 0; i < n; i++) {
		if (node[i]->parent == NULL) {
			if (*i1 == 0) {
				*i1 = i;
			}
			else if (*i2 == 0) {
				*i2 = i;
			}
			else {
				break;
			}
		}
	}

	if (node[*i1]->value > node[*i2]->value) {
		int temp = *i1;
		*i1 = *i2;
		*i2 = temp;
	}

	for (int i = 0; i < n; i++) {
		if (node[i]->parent == NULL) {

			if (node[i]->value < node[*i2]->value && i != *i1) {
				if (node[i]->value < node[*i1]->value) {
					*i2 = *i1; *i1 = i;
				}
				else {
					*i2 = i;
				}
			}
		}
	}
}

void CAppCompress::buildHuffmanTree(treeNode** node, int n) {
	int i1 = 0;
	int i2 = 0;
		
	select(node, &i1, &i2, n);

	node[i1]->parent = node[n];
	node[i2]->parent = node[n];
	node[n]->leftChild = node[i1];
	node[n]->rightChild = node[i2];
	node[n]->value = node[i1]->value + node[i2]->value;

	//TRACE("node[%d]: %d + node[%d]: %d = node[%d]: %d\r\n", i1, node[i1]->value, i2, node[i2]->value, n, node[n]->value);

}

int CAppCompress::DictBuild(unsigned char* pInput, int cDataSize, char *dict, int* lens){

	int colorValueShown[257];	// the times of every color shown
	treeNode* node[512];

	//init all values
	for (int i = 0; i < 257; i++) {
		colorValueShown[i] = 0;
		lens[i] = 0;
	}

	for (int i = 0; i < 512; i++) {
		treeNode* tempNode = new treeNode;
		tempNode->value = 0;
		tempNode->parent = NULL;
		tempNode->leftChild = NULL;
		tempNode->rightChild = NULL;
		node[i] = tempNode;
	}

	//calculate the times of every color shown
	for (int i = 0; i < cDataSize; i++) {
		unsigned char colorValue = pInput[i];
		int iColorValue = (unsigned int)colorValue;
		if (iColorValue < 0 || iColorValue >= 256) {
		}
		colorValueShown[iColorValue]++;
	}

	
	//build all leaf nodes
	int leafNum = 0;
	for (int i = 0; i < 256; i++) {
		if (colorValueShown[i] != 0) {
			node[leafNum]->key = i;
			node[leafNum]->value = colorValueShown[i];
			node[leafNum]->parent = NULL;
			node[leafNum]->leftChild = NULL;
			node[leafNum]->rightChild = NULL;
			leafNum++;
		}
	}

	//build the huffman tree
	for (int i = leafNum; i < 2*leafNum-1; i++) {
		buildHuffmanTree(node, i);
	}

	int max_len = 0;
	for (int i = 0; i < leafNum; i++) {
		int numParent = 0;
		//int code = 0;
		unsigned char code[32];
		for (int i = 0; i < 32; i++) {
			code[i] = '\0';
		}
		treeNode* parentNode = node[i]->parent;
		treeNode* childNode = node[i];
		while (childNode->parent != NULL) {
			if (childNode == parentNode->leftChild) {
				//code += pow(2, numParent);
				unsigned char temp = code[31 - numParent / 8] | (('1' - '0') << (numParent % 8));
				code[31 - numParent / 8] = temp;
			}
			numParent++;
			childNode = childNode->parent;
			parentNode = childNode->parent;
			
		}

		//dict[node[i]->key] = code;
		memcpy(dict+ node[i]->key*32, code, 32);
		lens[node[i]->key] = numParent;

		//TRACE("node[%d]: %d, code: %s, lens: %d\r\n", i, node[i]->value, code, numParent);
		if (numParent > max_len) {
			max_len = numParent;
		}
	}




	return max_len;

}
