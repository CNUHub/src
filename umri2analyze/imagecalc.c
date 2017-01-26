/*
From umn-cs!xiaoping%geronimo.drad.umn.edu Mon Oct 25 23:16:30 1993
Date: Mon, 25 Oct 93 17:24:20 CDT
From: "Xiaoping Hu" <umn-cs!xiaoping%geronimo.drad.umn.edu>
To: vapet!jtlee
Subject: example program
Content-Length: 5042
*/

/* IMAGECALC.C */

/******************************************************************************
   A. Rath, 11-27-90
******************************************************************************/

#include <sys/file.h>
#include <sys/errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
/* #include "data.h" (structure definitions are given explicitly below) */

struct datafilehead
{
    long nblocks;        /* number of blocks in file     */
    long ntraces;        /* number of traces per block   */
    long np;             /* number of elements per trace */
    long ebytes;         /* number of bytes per element  */
    long tbytes;         /* number of bytes per trace    */
    long bbytes;         /* number of bytes per block    */
    short transf;        /* transposed storage flag      */
    short status;        /* status of whole file         */
    long spare1;         /* reserved for future use      */
} main_header1, main_header2;

struct datablockhead
{
    short scale;         /* scaling factor               */
    short status;        /* status of data in block      */
    short index;         /* block index                  */
    short spare3;        /* reserved for future use      */
    long  ctcount;       /* completed transients in fids */
    float lpval;         /* left phase in phasefile      */
    float rpval;         /* right phase in phasefile     */
    float lvl;           /* level drift correction       */
    float tlt;           /* tilt drift correction        */
} block_header1, block_header2;

main(argc, argv)
int   argc;
char  *argv[];
{
    int     dim1, dim2, div_flag, i, in_file1, in_file2, out_file;
    double  factor;
    char    in_name1[100], in_name2[100], out_name[100];
    char    *malloc();
    float   *phasefile1, *phasefile2, *phasefileout;

    if (argc < 4  ||  argc > 5) {
        printf("Usage:  imagecalc phasefile1 phasefile2 outname [div, factor]\n");
	exit(1);
    }
    /****
    * Get input and output file names from command line arguments:
    ****/
    else {
        strcpy(in_name1, argv[1]);
        strcpy(in_name2, argv[2]);
        strcpy(out_name, argv[3]);
    }
    /****
    * Check remaining arguments, if any:
    ****/
    if (argc > 4) {
	if (!strcmp("div",argv[4])) {
	    div_flag = 1;
	}
        else {
	    div_flag = 0;
	    factor = atof(argv[4]);
	}
    }
    else
	factor = 1.0;


    /****
    * Open input files:
    ****/
    in_file1 = open(in_name1, O_RDONLY);
    if (in_file1 < 0) {
        perror("Open");
        printf("Can't open first input phasefile.\n");
        exit(4);
    }
    in_file2 = open(in_name2, O_RDONLY);
    if (in_file2 < 0) {
        perror("Open");
        printf("Can't open second input phasefile.\n");
        exit(4);
    }

    /****
    * Read main_headers.
    ****/
    if (read(in_file1, &main_header1, 32) != 32) {
        perror ("read");
        exit(5);
    }
    if (read(in_file2, &main_header2, 32) != 32) {
        perror ("read");
        exit(5);
    }

    /****
    * Check to see that matrix size is the same for both phasefiles:
    ****/
    if (main_header1.ntraces != main_header2.ntraces) {
        printf("Error:  Input phasefile dimensions do not match.\n");
        exit(4);
    }
    if (main_header1.np != main_header2.np) {
        printf("Error:  Input phasefile dimensions do not match.\n");
        exit(4);
    }
    dim1 = main_header1.ntraces;
    dim2 = main_header1.np;

    /****
    * Read block_headers.
    ****/
    if (read(in_file1, &block_header1, 28) != 28) {
        perror ("read");
        exit(5);
    }
    if (read(in_file2, &block_header2, 28) != 28) {
        perror ("read");
        exit(5);
    }

    /****
    * Allocate space for input data.
    ****/
    phasefile1 = (float *)malloc(dim1*dim2*sizeof(float));
    phasefile2 = (float *)malloc(dim1*dim2*sizeof(float));
    phasefileout = (float *)malloc(dim1*dim2*sizeof(float));

    /****
    * Read input phasefiles:
    ****/
    read(in_file1, phasefile1, dim1*dim2*sizeof(float));
    read(in_file2, phasefile2, dim1*dim2*sizeof(float));

    /****
    * Add or divide the two phasefiles:
    ****/
    if (div_flag) {
        for (i=0; i<dim1*dim2; i++) {
	    phasefileout[i] = phasefile1[i]/phasefile2[i];
        }
    }
    else {
        for (i=0; i<dim1*dim2; i++) {
	    phasefileout[i] = phasefile1[i] + factor*phasefile2[i];
        }
    }

    /****
    * Open output phasefile.
    ****/
    out_file = open(out_name, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (out_file < 0) {
        perror("open output file:");
        exit(7);
    }

    /****
    * Write out the main header.
    ****/
    write(out_file, &main_header1, 32);
    write(out_file, &block_header1, 28);
    write(out_file, phasefileout, dim1*dim2*sizeof(float));

    /****
    * Finished with data, so close everything up cleanly.
    ****/
    close(in_file1);
    close(in_file2);
    close(out_file);

    free(phasefile1);
    free(phasefile2);
    free(phasefileout);
}

