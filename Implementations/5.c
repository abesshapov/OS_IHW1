#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

const int BUFFER_SIZE = 5000;

void getIntersection(char* strings, char* output) {
    int ascii[128];
    int count = 0;
    int i;
    for (i = 0; i < strlen(strings); i++) {
        if (strings[i] == '\n') {
            i++;
            break;
        }
        ascii[(int)strings[i]]++;
    }

    for (; i < strlen(strings); i++) {
        if (ascii[(int)strings[i]] >= 1) {
            output[count++] = strings[i];
            ascii[(int)strings[i]] -= 5001;
        }
    }
    output[count] = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Incorrect amount of command line arguments: input and output files are needed.");
        return 0;
    }
    char *input_file = argv[1];
    char *output_file = argv[2];
    int read_fd, write_fd;
    read_fd = open(input_file, O_RDONLY);
    write_fd = open(argv[2], O_WRONLY | O_CREAT, 0666);
    if (read_fd == -1 || write_fd == -1) {
        printf("Input file is non-existnet.");
        return 0;
    }
    char* first_pipe = "first_pipe";
    char* second_pipe = "second_pipe";
    if (mkfifo(first_pipe, 0666) == -1 || mkfifo(second_pipe, 0666) == -1) {
        printf("Error occurred while creating pipes.");
        return 0;
    }
    char strings[BUFFER_SIZE], intersection[BUFFER_SIZE];
    pid_t first_pid, second_pid;
    first_pid = fork();
    if (first_pid == -1) {
        printf("Error occurred while creating processes.");
        return 0;
    }
    if (first_pid == 0) {
        int first_pipe_fd = open(first_pipe, O_WRONLY);
        char input_buffer[BUFFER_SIZE];
        size_t input_bytes_read = read(read_fd, input_buffer, BUFFER_SIZE);
        write(first_pipe_fd, input_buffer, input_bytes_read);
        close(first_pipe_fd);
        exit(0);
    }
    second_pid = fork();
    if (second_pid == -1) {
        printf("Error occurred while creating processes.");
        return 0;
    }
    if (second_pid == 0) {
        int first_pipe_fd = open(first_pipe, O_RDONLY);
        int second_pipe_fd = open(second_pipe, O_WRONLY);
        read(first_pipe_fd, &strings, BUFFER_SIZE);
        close(first_pipe_fd);
        getIntersection(strings, intersection);
        write(second_pipe_fd, intersection, strlen(intersection) + 1);
        close(second_pipe_fd);
        exit(0);
    }
    int second_pipe_fd = open(second_pipe, O_RDONLY);
    char output_buffer[BUFFER_SIZE];
    size_t output_bytes_read = read(second_pipe_fd, output_buffer, BUFFER_SIZE);
    close(second_pipe_fd);
    write(write_fd, output_buffer, output_bytes_read);
    close(read_fd);
    close(write_fd);
    unlink(first_pipe);
    unlink(second_pipe);
    return 0;
}