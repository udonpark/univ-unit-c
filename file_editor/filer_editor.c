/* This is my attempt by 
Yo Kogure 32134541 
Start date: 25/August/2021
End date: 27/August/2021 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>       
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

void perror(char message[]){ /* I made perror function myself to return, as stdin library is not allowed.
Please note that this is not perror() we used in labs, as this is not from stdin library.
This takes in a string, message, and prints out to terminal as error message while returning void */ 
    write(2, message, strlen(message));
}
void pmessage(char message[]){ /* When I want to print out any message to terminal, I use this. Although it returns
nothing, this will taken in a string(array of characters) and writes it to the terminal. */ 
    write(1, message, strlen(message));
}

/* referred to a discussion on ed on reading file: https://edstem.org/au/courses/6004/discussion/558040 */
int main(int argc, char *argv[]) 
/* This is a main function that takes in multiple arguments, does works as specified in the manual
and returns an integer whether it ended successfully or not. For high-level descriptions, please refer to 
documentation.txt which is attached. Number of argument input is stored as an integer argc, and 
each arguments are stored as *argv[]. For example, if I want to refer to the second argument as a string, then
we point to argv[2]. */ 
{
    char file_name[128] = "sample.txt";
    char *outfile = NULL;
    char *pfile = file_name;
    int bool_file_selected = 0;
    int bool_tail_mode = 0;
    int nlength = 10;
    /* I have declared and initialized variables/pointer that I required */

    int opt;
    while (optind < argc){ 
        /* It goes through all arguments, its numbers stored in argc and strings stored in argv; and we use switch statement
        to check whether they are one of the optional arguments. If they are, such as d, n or L, we catch following argument 
        accordingly using optarg, such as the case for n. It keeps looping until all are checked. */ 
        if((opt = getopt(argc, argv, "d:n:L")) != -1){
            switch(opt){
                case 'd': 
                    outfile = optarg; /* If destinatio directory is given, set it as outfile(initially NULL) */ 
                    break; 
                case 'n':
                    nlength = atoi(optarg); /* Convert string to integer and record it as length */ 
                    break;
                case 'L':
                    bool_tail_mode = 1; /* Turn this boolean on, 1 as True and 0 as False */ 
                    break;
                case '?': 
                    perror("parameter is missing; please refer to the manual\n");
                    exit(2); /* Print out error in cases where ? is returned */ 
                    break; 
            } 
        }
        else{
            /* If opt == -1, that means a return integer specifying that getopt failed(not in any of the candidates),
            that means that the path to the file has been selected. Since we assume that it is always the first element, 
            I set it as argv[1], first element. And well, when it is a random string and not a file, it will omit error anyway
            in the following code later on.*/ 
            pfile = argv[1]; 
            /*https://stackoverflow.com/questions/18079340/using-getopt-in-c-with-non-option-arguments?noredirect=1&lq=1
            This page has helped me a ton in differentiating the file name and optional arguments */
            bool_file_selected = 1;
            optind++;
        }
    }


    int fd = open(pfile, O_RDONLY); /* this will open file with the file_name, for read purpose*/
    if (fd < 0) /* if error occurs in opening, fd will be not 0*/
    {
        perror("Error: Failed to open the file\n"); /* Spit out error message, and exit(1), exit the program returning value 1*/ 
        exit(1);
    }

    char buffer[1024];
    int n = 0;
    int i = 0;
    char c;
    int bytes_read; 

    if (bool_tail_mode){ /* We perform this when -L has been selected */ 
        while ((bytes_read = read(fd, &c, 1))) {
            if (c == '\n'){
                n++;
            } 
        }
        n++; /* Count number of lines here, and lastly increment 1 to balance*/ 
        lseek(fd, 0, SEEK_SET); /* Set pointer to the beginning of the file, since we have just iterated to the end 
        For lseek instructions I have learnt from: https://stackoverflow.com/questions/28013009/how-to-find-out-if-offset-cursor-is-at-eof-with-lseek*/ 
        int count = 0;
        int offset = 0;
        int j = 0;
        if (n <= nlength){ /* if the -n is greater than the actual number of lines available, output the whole file*/
            while ((bytes_read = read(fd, &c, 1)) && (j < nlength)) {
                if (c == '\n'){
                    j++;
                }
                buffer[i] = c;
                i++; 
            }
        }
        else{ /* If -n is less than the lines existing, then we will find from the tail*/ 
            while ((bytes_read = read(fd, &c, 1)) != 0){
                if (c == '\n'){
                    count++; 
                }
                offset++; /* This is an offset to move from the beginning, in the final iteration */ 
                if (count == n - nlength){
                    break;
                }
            }
            lseek(fd, offset, SEEK_SET); /* Now, set the pointer to the corresponding line where
            we have breaked in the previous part, to fulfil our L condition */ 
            while ((bytes_read = read(fd, &c, 1))) {
                buffer[i] = c;
                i++; /* Set buffer to the array of characters, including \n.*/ 
            }
        }
    }
    else{ /* If -L is not selected, */ 
        while ((bytes_read = read(fd, &c, 1)) && (n < nlength)) { 
            if (c == '\n'){
                n++;
            } /* Keep looping and obtaining characters until either file is read until the end, OR, it has read number of 
            lines specified in -n. */ 
            buffer[i] = c;
            i++;
        }
    } /* At this point, buffer, whether -L is selected or not, has the strings that we want to OUTPUT, to either terminal or
    another file that we are to create. */ 


    if (outfile){ /* outfile is initially NULL, and only if when -d is input with corresponing directory that this piece is ran*/ 
        if (bool_file_selected){ /* if target file is selected and not sample.txt, I use basename() to convert absolute file path
        to a file name. This link has been helpful in helping me implement from <libgen.h>.
        Referred to: https://man7.org/linux/man-pages/man3/basename.3.html */
            pfile = basename(argv[1]);
            strcat(outfile, pfile); /* This will add directory + filename. */ 
        }
        else{
            strcat(outfile, "sample.txt"); /* referred to https://www.educative.io/blog/concatenate-string-c to
            concatenate string, directory name and a file name*/
        }
        int fd2 = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0664); /* Open new file if not existed */ 
        if (fd2 < 0){
            perror("Copy has been unsuccessful!\n");
            exit(2); /* When fd2 is -1, that means something wrong has occurred, i.e, filepath not right */ 
        }
        write(fd2, buffer, strlen(buffer));
        pmessage("Copy has been successful!\n");
        close(fd2); /* Copy to the destination directory file and close the file which we opened */ 
    }
    else{
        write(1, buffer, strlen(buffer)); 
        /* If destination is not specified, just print it out to the terminal , using 1. */ 
    }
    close(fd); /* Close the target file we opened*/ 
    exit(0); /* On successful completion, it returns value 0 as specified */
}