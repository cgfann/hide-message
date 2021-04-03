/**
 * FIXME
 *
 * 
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


int process_header(FILE *wav_file, FILE *new_wav_file, short *sample_size_ptr, int *num_samples_ptr);
int process_data(FILE* wave_file, FILE *new_wav_file, FILE *text_file, short sample_size, int num_samples, int num_lsb);


int main(int argc, char *argv[]) 
{
    FILE *wav_file;             /* .wav file read */
    FILE *new_wav_file;         /* to hide message */
    FILE *text_file;            /* message */
    short sample_size;          /* size of an audio sample (bytes) */
    int num_samples;            /* number of audio samples, includes all channels */  
    int num_lsb;                /* number of least significant bits contributing to the message per sample */
    int num_chars = 0;          /* number of characters in the message */
    char wav_file_name[256];
    char text_file_name[256];
    char new_wav_file_name[256];
    int args_ok = 0;
    int header_ok = 0;    

    args_ok = get_wav_args(argc, argv, &num_lsb, wav_file_name, text_file_name);
    if (!args_ok) {
        return 1;
    }

    /* if the original wave file name does not have the proper format */
    if (strcmp(wav_file_name + strlen(wav_file_name) - 4, ".wav")) {
        printf(".wav file %s is missing \".wav\" \n", wav_file_name);
        return 2;
    }
    /* new wave file has the name of the original file with _msg before the .wav extension */
    strcpy(new_wav_file_name, wav_file_name);
    strcpy((new_wav_file_name + strlen(new_wav_file_name) - 4), "_msg.wav");
    
    wav_file = fopen(wav_file_name, "rbe"); 
    if (!wav_file) {
        printf("could not open .wav file %s \n", wav_file_name);
        return 3;
    }

    new_wav_file = fopen(new_wav_file_name, "wbe");
    if(!new_wav_file) {
        printf("could not create .wav file %s \n", new_wav_file_name);
        return 3;
    }

    header_ok = process_header(wav_file, new_wav_file,  &sample_size, &num_samples);
    if (!header_ok) {
       printf(".wav file %s has incompatible format \n", wav_file_name);   
       return 4;
    }

    text_file = fopen(text_file_name, "re");
    if (!text_file) {
        printf("could not open .txt file %s \n", text_file_name);
        return 3;
    }

    num_chars = process_data(wav_file, new_wav_file, text_file, sample_size, num_samples, num_lsb);
    printf("%d characters written to %s \n", num_chars, new_wav_file_name);

    if (wav_file) {
        fclose(wav_file);
    }
    if (new_wav_file) {
        fclose(new_wav_file);
    }
    if (text_file) {
        fclose(text_file);
    }
    return 0;
}


/**
 *  function reads the RIFF, fmt, and start of the data chunk to collect information for reading/copying the data
 **/
int process_header(FILE *wav_file, FILE *new_wav_file, short *sample_size_ptr, int *num_samples_ptr) 
{
    char chunk_id[] = "    ";  /* chunk id */
    char data_4[] = "    ";    /* chunk data, 4 bytes */
    int chunk_size = 0;        /* number of bytes in chunk */
    short audio_format = 0;    /* audio format type, PCM = 1 */
    short num_channels = 0;    
    short bits_per_smp = 0; 
    int data_bytes = 0;        /* number of bytes of data in all samples*/

    /* copy over information from wav_file to new_wav_file */
    fread(chunk_id, 4, 1, wav_file);
    fwrite(chunk_id, 4, 1, new_wav_file);

    fread(&chunk_size, 4, 1, wav_file);
    fwrite(&chunk_size, 4, 1, new_wav_file);

    fread(data_4, 4, 1, wav_file);
    fwrite(data_4, 4, 1, new_wav_file);

    fread(chunk_id, 4, 1, wav_file);
    while(strcmp(chunk_id, "fmt ") != 0) {
        fread(&chunk_size, 4, 1, wav_file);
        fseek(wav_file, chunk_size, SEEK_CUR);
        fread(chunk_id, 4, 1, wav_file);
    }
    fwrite(chunk_id, 4, 1, new_wav_file); 

    fread(&chunk_size, 4, 1, wav_file);
    fwrite(&chunk_size, 4, 1, new_wav_file);

    fread(&audio_format, sizeof(audio_format), 1, wav_file);
    fwrite(&audio_format, sizeof(audio_format), 1, new_wav_file);

    fread(&num_channels, sizeof(num_channels), 1, wav_file);
    fwrite(&num_channels, sizeof(num_channels), 1, new_wav_file);

    fseek(wav_file, 10, SEEK_CUR);                            

    fread(&bits_per_smp, sizeof(bits_per_smp), 1, wav_file);
    fwrite(&bits_per_smp, sizeof(bits_per_smp), 1, new_wav_file);

    fread(&chunk_id, 4, 1, wav_file);
    while(strcmp(chunk_id, "data") != 0) {
        fread(&chunk_size, 4, 1, wav_file);        
        fseek(wav_file, chunk_size, SEEK_CUR);
        fread(chunk_id, 4, 1, wav_file);
    }
    fwrite(&chunk_id, 4, 1, new_wav_file);

    fread(&data_bytes, sizeof(data_bytes), 1, wav_file);
    fwrite(&data_bytes, sizeof(data_bytes), 1, new_wav_file);
    
    *sample_size_ptr = bits_per_smp / 8;                    /* sample size in bytes */
    *num_samples_ptr = data_bytes / (*sample_size_ptr);     /* total number of audio samples */ 

    return (audio_format == 1);
}


/**
 *  function reads, modifies, and writes samples to and from a .wav file, hiding a message in the LSBs of the new samples
 **/
int process_data(FILE *wav_file, FILE *new_wav_file, FILE *text_file, short sample_size, int num_samples, int num_lsb) 
{
    unsigned char msg_bits = 0;        /* sequence of bits from the message to be written into new_wav_file samples */
    int sample = 0;
    int new_sample = 0;                /* sample with modified least significant bits, to be written into new_wav_file */
    unsigned int mask = 0;  
    unsigned char smiley[] = ":)";
    unsigned char ch = 0;
    int num_samples_written = 0;
    int num_chars = 0;           
    int i;
    int j;

    switch (num_lsb) {
    case 1:
        mask = 0x1; break;    /* .... 0001 */
    case 2:
        mask = 0x3; break;    /* .... 0011 */
    case 4:
        mask = 0xf; break;   /* .... 1111 */
    default:
        break;
    }

    ch = fgetc(text_file);
    /* leave enough samples to encrypt ":)" even if the message has to be truncated. */
    while ((ch != EOF) && (num_samples_written < (num_samples - (16 / num_lsb)))) {
        num_chars++;
        for (i = 8 - num_lsb; i > 0; i -= num_lsb) {
            msg_bits = ch & (mask << i);                         /* extract particular bits from message character*/

            fread(&sample, sample_size, 1, wav_file);
            if (sample_size == 2) {
                sample = (short)sample;                
            }     
            new_sample = (~mask & sample) | (msg_bits >> i);     /* turn off LSBs from original sample, place message bits */
            if (sample_size == 2) {
                new_sample = (short)new_sample;
            }
            fwrite(&new_sample, sample_size, 1, new_wav_file);
            num_samples_written++;
        }
            ch = fgetc(text_file);
    }
    for (i = 0; i < 2; i++) {
        ch = smiley[i];
        num_chars++;
        for (j = 8 - num_lsb; j > 0; j -= num_lsb) {
            msg_bits = ch & (mask << j);                         /* extract particular bits from message character */

            fread(&sample, sample_size, 1, wav_file);
            if (sample_size == 2) {
                sample = (short)sample;                 
            }     
            new_sample = (~mask & sample) | (msg_bits >> j);     /* turn off LSBs from original sample, place message bits */
            if (sample_size == 2) {
                new_sample = (short)new_sample;
            }
            fwrite(&new_sample, sample_size, 1, new_wav_file);
            num_samples_written++;
        }
    }

    /* copy the rest of the data */
    while (num_samples_written < num_samples) {
        fread(&sample, sample_size, 1, wav_file);
        fwrite(&sample, sample_size, 1, new_wav_file);
        num_samples_written++;                                     
    }
    return num_chars;
}
