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
	cDataSize = width * height * 3;
	// Create Variable @var{compressedData}
	int size_compressed_data = 1024;
	unsigned char *compressedData = new unsigned char[size_compressed_data];
	memset(compressedData, 0, sizeof(unsigned char) * size_compressed_data);
	/* Store the position where we writing to @var{compressedData} */
	int pos_data = 1;

	/*
	 * Huffman Tree Table;
	 */
	unsigned char* huffmanTree = new unsigned char[256 * 32];
	memset(huffmanTree, 0, sizeof(unsigned char) * 256 * 32);

	/*
	 * Encoding length of Huffman Code
	 */
	int *encoding_len = new int[256];
	memset(encoding_len, 0, sizeof(int) * 256);

	/* Build Huffman Tree */
	unsigned int encoding_max_len = DictBuild(pInput, cDataSize, huffmanTree, encoding_len);
	encoding_max_len = calcComplementLength(encoding_max_len);
	int byte_encoding_max = encoding_max_len / 8;

	/**
	 * Package the Huffman Table into @var{compressedData}
	 */
	unsigned int treeSize = 0;
	for (unsigned int i = 0; i < 256; ++i) {
		if (encoding_len[i] != 0) {
			++treeSize;
			// Put the key of Huffman Table
			writeData(compressedData, size_compressed_data, ++pos_data, (unsigned char)i);
			// Put the value of Huffman Table
			for (int j = 32 - byte_encoding_max; j < 32; ++j) {
				writeData(compressedData, size_compressed_data, ++pos_data, huffmanTree[i * 32 + j] & 0xFF);
			}
			// Put the encoding len of each Huffman code
			writeData(compressedData, size_compressed_data, ++pos_data, (unsigned char)encoding_len[i]);
		}
	}
	/**
	 * Store the size of Huffman Table
	 * &
	 * bytes of max encoding len of Huffman Table
	 * at the head @var{compressedData}
	 */
	compressedData[0] = (unsigned char)treeSize;
	compressedData[1] = (unsigned char)byte_encoding_max;

	// Create Encoded Sequence
	unsigned char *encodedSequence = new unsigned char[cDataSize * byte_encoding_max];
	memset(encodedSequence, 0, sizeof(unsigned char) * cDataSize * byte_encoding_max);
	unsigned int *encodedLength = new unsigned int[cDataSize * byte_encoding_max];
	memset(encodedLength, 0, sizeof(unsigned int) * cDataSize * byte_encoding_max);

	// Create a char*[32] to store a huffman code
	unsigned char* huffmanCode = new unsigned char[32];
	unsigned char* encoded = new unsigned char[byte_encoding_max];

	// pmy => primary, means one of the three-primary-color, like (R, G, B)
	for (int pmy = 0; pmy < cDataSize; ++pmy) {
		// Empty chars
		memset(encoded, 0, sizeof(unsigned char) * byte_encoding_max);
		// Get mapped Huffman Code
		for (int i = 0; i < 32; ++i) {
			huffmanCode[i] = huffmanTree[(pInput[pmy] * 32) + i] & 0xFF;
		}

		// Get mappedHuffman Code Length
		int len = encoding_len[pInput[pmy]];
		for (int i = 31, j = byte_encoding_max, _len = len; _len > 0 && i >= 0 && j >= 0; --i) {
			unsigned int l = _len > 8 ? 8 : _len;
			_len -= l;
			encoded[--j] = huffmanCode[i] & ((1 << l) - 1);
			// Save the encoded bits
			encodedSequence[pmy * byte_encoding_max + j] = encoded[j];
			// Save the length of encoded bits
			encodedLength[pmy * byte_encoding_max + j] = l;
		}
	}

	// Free Memory of Huffman
	delete[] huffmanTree;
	delete[] huffmanCode;
	delete[] encoded;

	// Using a Buffer to store the bits before getting 8 bits
	unsigned char buf;

	/* Current bits stored in Buffer */
	unsigned int pos_buf = 0;

	/* Current pointing Position of encodedSequence */
	unsigned int pos_seq = 0;

	unsigned char code = '\0';
	unsigned int len = 0;
	unsigned char code_pending = '\0';
	int seat_pending = -1;

	// Write to @var{CompressedData} from @var{encodedSequence}
	while (pos_buf < 8 || pos_seq < (cDataSize * byte_encoding_max)) {
		// Set Buffer position to 0
		pos_buf = 0;
		// Make Buffer Empty
		buf = 0b0;
		// Loading bits to Buffer until fullfilled it
		while (pos_buf < 8) {
			// Check whether there is some bits pending
			if (seat_pending > -1) {
				code = code_pending;
				len = seat_pending;

				seat_pending = -1;
			}
			else if (pos_seq >= cDataSize * byte_encoding_max) {
				// Finsihed reading @var{encodedSequence]
				pos_buf = 8;
				break;
			}
			else {
				code = encodedSequence[pos_seq];
				len = encodedLength[pos_seq];
				++pos_seq;
			}

			if (pos_buf + len <= 8) {
				/* calculate how many bit to move */
				int bit_to_move = 9 - (pos_buf + 1) - len;

				buf = buf | (code << bit_to_move);
				pos_buf += len;
			}
			else {
				/* calculate how many seat have in the Buffer */
				int empty_seat = 8 - pos_buf;

				// divide the code into two parts
				// one push into Buffer
				// the other will be using at next iteration
				seat_pending = len - empty_seat;
				buf = buf | (code >> seat_pending);
				code_pending = code & ((1 << seat_pending) - 1);
				pos_buf += empty_seat;
			}
		}
		writeData(compressedData, size_compressed_data, ++pos_data, buf & 0xFF);
	}

	cDataSize = pos_data + 1;

	// Free Memory Space
	delete[] encodedSequence;
	delete[] encodedLength;
	//delete[] encoding_len;

	return compressedData;		// return the compressed data
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {
	// Writing position of uncompressedData
	unsigned int pos_uncompressed_data = 0;
	// Reading position of compressedData
	int pos_data = -1;
	// Huffman Tree Table Size
	int treeSize = compressedData[++pos_data] & 0xFF;
	// Maximun Length of Encoding
	int byte_encoding_max = compressedData[++pos_data] & 0xFF;

	unsigned char* huffmanTree = new unsigned char[256 * 32];
	memset(huffmanTree, 0, sizeof(unsigned char) * 256 * 32);
	int *encoding_len = new int[256];
	memset(encoding_len, 0, sizeof(unsigned int) * 256);
	for (int i = 0; i < treeSize && pos_data < cDataSize; ++i) {
		// Recover the key of Huffman Table
		unsigned int key = compressedData[++pos_data] & 0xFF;
		// Recover the value of Huffman Table
		for (int j = 32 - byte_encoding_max; j < 32; ++j) {
			huffmanTree[key * 32 + j] = (compressedData[++pos_data] & 0xFF);
		}
		// Recover the encoding length of Huffman Table
		encoding_len[key] = (int)(compressedData[++pos_data] & 0xFF);
	}

	// Rebuild Huffman Tree
	treeNode* rootNode = treeBuild(huffmanTree, encoding_len);

	unsigned char buf = compressedData[++pos_data] & 0xFF;
	unsigned int pos_buf = 0;
	while (pos_data < cDataSize) {
		treeNode* node = rootNode;
		while (1) {
			if (pos_buf > 7) {
				buf = compressedData[++pos_data] & 0xFF;
				pos_buf = 0;
			}
			if (((buf >> (7 - pos_buf)) & 0b1) == 1) {
				if (node->leftChild != nullptr) {
					node = node->leftChild;
				} else {
					++pos_buf;
					break;
				}
			}
			else {
				if (node->rightChild != nullptr) {
					node = node->rightChild;
				} else {
					++pos_buf;
					break;
				}
			}
			++pos_buf;
		}
		//TRACE("%d, %d %d\n", pos_data, cDataSize, pos_uncompressed_data);
		// Write to uncompressedData
		uncompressedData[pos_uncompressed_data++] = node->key & 0xFF;
	}

	// Free Memory
	delete[] huffmanTree;
	delete[] encoding_len;
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

void InsertNode(treeNode** node, treeNode* newNode) {
	for (int i = 2; i < 256; i++) {
		if (newNode->value < node[i]->value || node[i]->value == 0) {
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

void buildHuffmanTree(treeNode** node, int n) {
	int i1 = 0;
	int i2 = 0;

	select(node, &i1, &i2, n);

	node[i1]->parent = node[n];
	node[i2]->parent = node[n];
	node[n]->leftChild = node[i1];
	node[n]->rightChild = node[i2];
	node[n]->value = node[i1]->value + node[i2]->value;
}

int DictBuild(unsigned char* pInput, int cDataSize, unsigned char* dict, int* lens) {

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
	for (int i = leafNum; i < 2 * leafNum - 1; i++) {
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
		memcpy(dict + node[i]->key * 32, code, 32);
		lens[node[i]->key] = numParent;

		if (numParent > max_len) {
			max_len = numParent;
		}
	}

	return max_len;

}

treeNode* treeBuild(unsigned char* dict, int* lens) {

	treeNode* nodes[512];
	for (int i = 0; i < 512; i++) {
		treeNode* tempNode = new treeNode;
		tempNode->key = -1;
		tempNode->parent = NULL;
		tempNode->leftChild = NULL;
		tempNode->rightChild = NULL;
		nodes[i] = tempNode;
	}
	int currentNum = 0;
	treeNode* rootNode = nodes[0];
	treeNode* currentNode = rootNode;
	for (int i = 0; i < 256; i++) {
		if (lens[i] != 0) {
			int len = lens[i];
			for (int j = 32 * 8 - len; j < 32 * 8; j++) {
				if ((dict[32 * i + j / 8] >> (7 - j % 8)) & ('1' - '0') == ('1' - '0')) {
					if (currentNode->leftChild == NULL) {
						currentNode->leftChild = nodes[currentNum + 1];
						currentNum = currentNum + 1;
					}
					currentNode = currentNode->leftChild;
				}
				else {
					if (currentNode->rightChild == NULL) {
						currentNode->rightChild = nodes[currentNum + 1];
						currentNum = currentNum + 1;
					}
					currentNode = currentNode->rightChild;
				}
			}
			currentNode->key = i;
			currentNode = rootNode;
		}
	}

	return rootNode;
}


/* Calculate the length that complement binary bit */
int calcComplementLength(int len) {
	int n = 0;
	while (++n <= 32) {
		if (len < n * 8) return n * 8;
	}
	return 256;
}

/* Automatically adjust size of unsigned char* */
void writeData(unsigned char* &target, int& size, int pos, unsigned char data) {
	if (pos + 1 > size) {
		unsigned char* base = target;
		int adjustedSize = int(size * 1.1);
		base = new unsigned char[adjustedSize];
		memcpy(base, target, size);
		delete[] target;
		size = adjustedSize;
		target = base;
		TRACE("[MEMORY]: %d MB\n", (int)(adjustedSize / 1024 / 1024));
	}
	target[pos] = data;
}
