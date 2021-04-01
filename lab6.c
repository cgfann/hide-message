/**
 * FIXME
 *
 * @author Charlotte Fanning {@literal <fanncg18@wfu.edu>}
 * @date April 5, 2021
 * @assignment Lab 6  
 * @course CSC 250
 **/


#include "get_wav_args.h"
#include <math.h>  
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  


int read_wav_header(FILE *in_file, FILE *new_wav_file, short *sample_size_ptr, int *num_samples_ptr);
void read_wav_data(FILE* in_file, char out_name[], short sample_size, int num_samples, int bits);


int main(int argc, char *argv[]) 
{
    FILE *wav_file;     /* .wav file read */
    FILE *new_wav_file; /* to contain message */
    short sample_size;  /* size of an audio sample (bytes) */
    int num_samples;    /* number of audio samples, includes all channels */ 
    int wav_ok = 0;     
    int bits;           /* number of least significant bits contributing to the message per sample*/
    char wav_file_name[256];
    char text_file_name[256];
    char new_wav_file_name[256];
    int args_ok;

    args_ok = get_wav_args(argc, argv, &bits, wav_file_name, text_file_name);
    if (!args_ok) {
        printf("exiting...");
        return 1;
    }

    if (strcmp(wav_file_name + strlen(wav_file_name) - 4, ".wav")) {
        printf(".wav file %s is missing \".wav\" \n", wav_file_name);
        return 2;
    }
    strcpy(new_wav_file_name, wav_file_name);
    strcpy((new_wav_file_name + strlen(new_wav_file_name) - 4), "_msg.wav");
    
    wav_file = fopen(wav_file_name, "rbe"); 
    if (!wav_file) {
        printf("could not open wav file %s \n", argv[3]);
        return 2;
    }

    new_wav_file = fopen(new_wav_file_name, "wbe");
    if(!new_wav_file) {
        printf("could not create .wav file %s \n", new_wav_file_name);
        return 2;
    }
    
    wav_ok = read_wav_header(wav_file, new_wav_file, &sample_size, &num_samples);
    if (!wav_ok) {
       printf("wav file %s has incompatible format \n", argv[1]);   
       return 3;
    }

    /* read_wav_data(in_file, text_file_name, sample_size, num_samples, bits); */

    if (wav_file) {
        fclose(wav_file);
    }
    if (new_wav_file) {
        fclose(new_wav_file);
    }
    return 0;
}


/**
 *  function reads the RIFF, fmt, and start of the data chunk to collect information for reading the data
 **/
int read_wav_header(FILE *wav_file, FILE *new_wav_file, short *sample_size_ptr, int *num_samples_ptr) 
{
    char chunk_id[] = "    ";  /* chunk id, note initialize as a C-string */
    char data_4[] = "    ";    /* chunk data, 4 bytes */
    char data_6[] = "      ";  /* chuck data, 6 bytes */
    char data_1[] = " ";
    int chunk_size = 0;        /* number of bytes remaining in chunk */
    short audio_format = 0;    /* audio format type, PCM = 1 */
    short num_channels = 0;    /* number of audio channels */ 
    int sample_rate = 0;
    short bits_per_smp = 0; 
    int num_samples = 0;
    int i;

    /* copy over information from wav_file to new_wav_file */
    fread(chunk_id, 4, 1, wav_file);
    fwrite(chunk_id, 4, 1, new_wav_file);

    fread(&chunk_size, 4, 1, wav_file);
    fwrite(&chunk_size, 4, 1, new_wav_file);

    fread(data_4, 4, 1, wav_file);
    fwrite(data_4, 4, 1, new_wav_file);

    fread(chunk_id, 4, 1, wav_file);
    fwrite(chunk_id, 4, 1, new_wav_file); 

    while(strcmp(chunk_id, "fmt ") != 0) {
        
        fread(&chunk_size, 4, 1, wav_file);
        fwrite(&chunk_size, 4, 1, new_wav_file);

        for (i = 0; i < chunk_size; i++) {
            fread(data_1, 1, 1, wav_file);          /* byte by byte */
            fwrite(data_1, 1, 1, new_wav_file);
        }

        fread(chunk_id, 4, 1, wav_file);
        fwrite(chunk_id, 4, 1, new_wav_file); 
    }

    fread(&chunk_size, 4, 1, wav_file);
    fwrite(&chunk_size, 4, 1, new_wav_file);

    fread(&audio_format, sizeof(audio_format), 1, wav_file);
    fwrite(&audio_format, sizeof(audio_format), 1, new_wav_file);

    fread(&num_channels, sizeof(num_channels), 1, wav_file);
    fwrite(&num_channels, sizeof(num_channels), 1, new_wav_file);

    fread(&sample_rate, sizeof(sample_rate), 1, wav_file);
    fwrite(&sample_rate, sizeof(sample_rate), 1, new_wav_file);

    fread(data_6, 6, 1, wav_file);
    fwrite(data_6, 6, 1, new_wav_file);

    fread(&bits_per_smp, 1, sizeof(bits_per_smp), wav_file);
    fwrite(&bits_per_smp, 1, sizeof(bits_per_smp), new_wav_file);

    fread(&chunk_id, 1, 4, wav_file);
    fwrite(&chunk_id, 1, 4, new_wav_file);

    while(strcmp(chunk_id, "data") != 0) {
        fread(&chunk_size, 4, 1, wav_file);
        fwrite(&chunk_size, 4, 1, new_wav_file);
        
        fread(data_4, chunk_size, 1, wav_file);
        fwrite(data_4, chunk_size, 1, new_wav_file);
    
        fread(chunk_id, 4, 1, wav_file);
        fwrite(chunk_id, 4, 1, new_wav_file);
    }

    fread(&num_samples, 1, sizeof(num_samples), wav_file);  /* total bytes of data*/
    fwrite(&num_samples, 1, sizeof(num_samples), new_wav_file);
    
    *sample_size_ptr = bits_per_smp / 8;                    /* sample size in bytes */
    *num_samples_ptr = num_samples / (*sample_size_ptr);    /* total number of audio samples */ 

    return (audio_format == 1);

}


/**
 *  function reads the WAV audio data (last part of the data chunk) to decode message in the least significant bits
 **/
void read_wav_data(FILE *in_file, char out_name[], short sample_size, int num_samples, int bits) 
{
    FILE *out_file;
    unsigned int sample = 0;
    unsigned char last = 0; /* sequence of bits from a sample that are part of the message character */
    unsigned int mask = 0;  
    unsigned char prev_ch = 0;
    unsigned char ch = 0;
    int msg_samples = 0;    /* number of samples used to hide the message */
    int num_chars = 0;      /* number of characters in the message */
    int end = 0;            /* true when ":)" is encountered */
    int i;

    switch (bits) {
    case 1:
        mask = 0x1; break;    /* .... 0001 */
    case 2:
        mask = 0x3; break;    /* .... 0011 */
    case 4:
        mask = 0xf; break;   /* .... 1111 */
    default:
        break;
    }

    out_file = fopen(out_name, "wbe");

    /* until ":)" or the end of the file is reached */
    while (!end && (msg_samples < num_samples)) { 
        ch = 0;                                              /* reset ch buffer to read next encrypted character */
        for (i = 8; i > 0; i -= bits) {
            fread(&sample, sample_size, 1, in_file);         /* get next sample */
            if (sample_size == 2) {
                sample = (unsigned short)sample;
            }           

            last = (sample & mask) << (unsigned int)(i - bits);
            ch = ch | last;                                  /* append lsb of current sample to working bit sequence of ch */
            msg_samples++;
        } 
        num_chars++;
        fprintf(out_file, "%c", ch);           
             
        if ((prev_ch == ':') && (ch == ')')) {
            end = 1;
        }
        prev_ch = ch;
    }
    if (!end) {
          fprintf(out_file, "%s", ":)");    
    }

    printf("recovered %d characters from %d samples \n", num_chars, msg_samples);

    fclose(out_file);
}
