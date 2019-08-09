/* *********************************************************************************

	EXAMPLE main() for smbPitchShift using MiniAiff to handle sound file i/o
	(c) 2003 S. M. Bernsee, http://www.dspdimension.com
	
	IMPORTANT: requires miniAIFF to be included in the project

********************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MiniAiff.h"

#ifndef _MAX_PATH
    #define _MAX_PATH 1024
#endif


#define BUF_LENGTH	8192

void smbPitchShift(float pitchShift, long numSampsToProcess, long fftFrameSize, long osamp, float sampleRate, float *indata, float *outdata);

int main(void)
{
	static float data[BUF_LENGTH];
	int ret = 0;
	
	long semitones = 3;							// shift up by 3 semitones
	float pitchShift = pow(2., semitones/12.);	// convert semitones to factor
	
	char inFileName[_MAX_PATH];
	char outFileName[_MAX_PATH];

	printf("Running MiniAiff version %s\n(C) S.M.Bernsee - http://www.dspdimension.com\n\n", mAiffGetVersion());

	// Prompt the user for input and output files
	if (mAiffGetFilename(inFileName) == 0)		exit(-1);
	if (mAiffPutFilename(outFileName) == 0)		exit(-1);

	// Create output file
	printf("Creating output\n");
	ret = mAiffInitFile(outFileName);
	if (ret < 0) {
		printf("! Error creating output file %s - error #%d\n! File may be busy\n", outFileName, ret);
		exit(-1);
	}

	unsigned long inPosition=0;
	for(;;)
	{
	
		// Read from input file
		printf("Reading from input: %s\n", inFileName);
		ret = mAiffReadData(inFileName, data, inPosition, BUF_LENGTH);
		if (ret < 0) {
			printf("! Error reading input file %s - error #%d\n! Wrong format or filename\n", inFileName, ret);
			exit(-1);
		} else if (ret == 0) break;		// end of file reached
		
		// Increment the read position
		inPosition += BUF_LENGTH;

		// --------------------------------- call smbPitchShift() ---------------------------------
		smbPitchShift(pitchShift, BUF_LENGTH, 2048, 4, mAiffGetSampleRate(inFileName), data, data);
		// ----------------------------------------------------------------------------------------

		// write processed data to output file
		printf("Writing to output: %s\n", outFileName);
		ret = mAiffWriteData(outFileName, data, BUF_LENGTH);
		if (ret < 0) {
			printf("! Error writing output file %s - error #%d\n! Not enough space?\n", outFileName, ret);
			exit(-1);
		}
	}

	// That's all there is to it.
	printf("You're a cool dude now!\n");
	return 0;
}

