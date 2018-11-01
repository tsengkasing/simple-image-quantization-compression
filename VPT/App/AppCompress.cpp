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

	cDataSize = width * height * 3 ;				// You need to determine the size of the compressed data. 
													// Here, we simply set it to the size of the original image
	compressedData = new unsigned char[cDataSize] ; // As an example, we just copy the original data as compressedData.

	memcpy(compressedData, pInput, cDataSize) ;

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
