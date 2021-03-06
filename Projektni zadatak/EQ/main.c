//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"

#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];

Int16 dirakBuff[AUDIO_IO_SIZE];
Int16 izlaz[AUDIO_IO_SIZE];

Int16 outputShalvingHP[AUDIO_IO_SIZE];
Int16 outputShalvingLP[AUDIO_IO_SIZE];

Int16 outputShalvingHP_Coeffs[4];
Int16 outputShalvingLP_Coeffs[4];
Int16 outputPeek_Coeffs[6];

Int16 history_x[AUDIO_IO_SIZE];
Int16 history_y[AUDIO_IO_SIZE];

void main(void) {
	int i;

	/* Inicijalizaija razvojne ploce */
	EZDSP5535_init();

	/* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
	EZDSP5535_SAR_init();

	/* Inicijalizacija LCD kontrolera */
	initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

	/* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
	aic3204_hardware_init();

	/* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

	aic3204_dma_init();

	dirakBuff[0] = 16000;
	history_x[0] = 0;
	history_y[0] = 0;

	for (i = 1; i < AUDIO_IO_SIZE; i++) {
		dirakBuff[i] = 0;
		history_x[i] = 0;
		history_y[i] = 0;
	}

	/* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
	set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

	calculateShelvingCoeff(-0.3, outputShalvingHP_Coeffs);
	calculateShelvingCoeff(0.3, outputShalvingLP_Coeffs);

	calculatePeekCoeff(0.7, 0, outputPeek_Coeffs);

	while (1) {
		aic3204_read_block(sampleBufferL, sampleBufferR);

		/* Your code here */
		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			outputShalvingHP[i] = shelvingHP(dirakBuff[i],
					outputShalvingHP_Coeffs, history_x, history_y, 8192);
		}

		for (i = 0; i < AUDIO_IO_SIZE; i++) {
			outputShalvingLP[i] = shelvingLP(dirakBuff[i],
					outputShalvingLP_Coeffs, history_x, history_y, 8192);
		}

		aic3204_write_block(sampleBufferR, sampleBufferR);
	}

	/* Prekid veze sa AIC3204 kodekom */
	aic3204_disable();

	printf("\n***Kraj programa***\n");
	SW_BREAKPOINT
	;
}
