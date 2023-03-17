#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

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
    char strings[BUFFER_SIZE], intersection[BUFFER_SIZE], result[BUFFER_SIZE];
    int first_pipe[2], second_pipe[2];
    pid_t first_pid, second_pid, third_pid;
    if (pipe(first_pipe) == -1 || pipe(second_pipe) == -1) {
        printf("Error occurred while creating pipes.");
        return 0;
    }
    first_pid = fork();
    if (first_pid == -1) {
        printf("Error occurred while creating processes.");
        return 0;
    }
    if (first_pid == 0) {
        close(first_pipe[0]);
        close(second_pipe[0]);
        close(second_pipe[1]);
        char input_buffer[BUFFER_SIZE];
        size_t input_bytes_read = read(read_fd, input_buffer, BUFFER_SIZE);
        write(first_pipe[1], input_buffer, input_bytes_read);
        close(first_pipe[1]);
        exit(0);
    }
    second_pid = fork();
    if (second_pid == -1) {
        printf("Error occurred while creating processes.");
        return 0;
    }
    if (second_pid == 0) {
        close(first_pipe[1]);
        close(second_pipe[0]);
        read(first_pipe[0], &strings, BUFFER_SIZE);
        close(first_pipe[0]);
        getIntersection(strings, intersection);
        write(second_pipe[1], intersection, strlen(intersection) + 1);
        close(second_pipe[1]);
        exit(0);
    }
    third_pid = fork();
    if (third_pid == -1) {
        printf("Error occurred while creating processes.");
        return 0;
    }
    if (third_pid == 0) {
        close(first_pipe[0]);
        close(first_pipe[1]);
        close(second_pipe[1]);
        size_t output_bytes_read = read(second_pipe[0], result, BUFFER_SIZE);
        close(second_pipe[0]);
        write(write_fd, result, output_bytes_read);
        close(read_fd);
        close(write_fd);
        exit(0);
    }
    close(first_pipe[0]);
    close(first_pipe[1]);
    close(second_pipe[1]);
    close(second_pipe[0]);
    close(read_fd);
    close(write_fd);
    waitpid(first_pid, NULL, 0);
    waitpid(second_pid, NULL, 0);
    waitpid(third_pid, NULL, 0);
    return 0;
}