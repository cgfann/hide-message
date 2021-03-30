/**
 * @author Nirre Pluf {@literal pluf@wfu.edu}
 * @date Mar. 11, 2021
 * @assignment Lab 5
 * @course CSC 250
 *
 * source files for parsing the command line arguments
 **/   
#include "get_wav_args.h"

/**
 *  function processes the command line arguments using getopt()
 *
 *  function returns 1 if the arguments are "correct", 0 otherwise
 *
 *  argc - number of command line arguments
 *  argv - list of command line arguments (list of C-strings)
 *  bit_ptr - pointer to integer, number of LSB bits (1, 2, or 4)
 *  wav_file_name - pointer to the WAV filename  
 *  text_file_name - pointer to the filename that will store the hiden text 
 */
int get_wav_args(int argc, char *argv[], int* bit_ptr, char wav_file_name[], char text_file_name[]) {
    int opt;
    int got_bit = 0;

    while ((opt = getopt(argc, argv, "b:")) != -1) {
        switch (opt) {
        case 'b':
            *bit_ptr = atoi(optarg);
            got_bit = *bit_ptr == 1 || *bit_ptr == 2 || *bit_ptr == 4;
            break;
        default:
            print_command_usage(argv[0]);
            return 0;
        }
    }

    if(!got_bit || (optind + 2) != argc){
        print_command_usage(argv[0]);
        return 0;
    }

    strcpy(wav_file_name, argv[optind]);
    strcpy(text_file_name, argv[optind + 1]);

    /* 
    printf("b: %d, wav_f: [%s], txt_f:[%s] \n", *bit_ptr, wav_file_name, text_file_name); 
    */

    return 1;
}


/**
 *  function to print a useful help message
 */
void print_command_usage(char executable_name[]) {
    fprintf(stderr, "Usage: %s -b bit wav_file_name text_file_name \n", executable_name);
    fprintf(stderr, " -b bits per sample to encode (1, 2, or 4)\n");
}




