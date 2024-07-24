#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/statvfs.h>

#define BUFFER_SIZE 409600
#define COUNT 12

using namespace std;

double ReadB = 0;
double WriteB = 0;
double lastWriteB = -1;

struct aio_operation
{
    struct aiocb aio;
    //char *buffer;
    int write_operation;
    struct aio_operation *next_operation;
    int num;
    bool end;
};

void aio_completion_handler(sigval_t sigval)
{
    struct aio_operation *aio_op = (struct aio_operation *)sigval.sival_ptr;
    struct aio_operation *next_op = aio_op->next_operation;

    (aio_op->aio).aio_offset += BUFFER_SIZE * COUNT;

    if (aio_op->write_operation)
    {
        ssize_t c = aio_return(&aio_op->aio);
        WriteB = WriteB + c;
        aio_read(&next_op->aio);
    }
    else
    {
        ssize_t c = aio_return(&aio_op->aio);
        ReadB = ReadB + c;
        if (c != 0)
        {
            next_op->aio.aio_nbytes = c;
            aio_write(&next_op->aio);
        }
        else
        {
            aio_op->end = true;
        }
    }
}

int main()
{
    const char *sourceFile = "text_5GB.txt";
    const char *destFile = "test.txt";

    double execTime = 0;
    int execTimeInt = 0;

    int sourceFd = open(sourceFile, O_RDONLY | O_NONBLOCK, 0666);
    if (sourceFd == -1) {
        perror("Error with source file");
        return 1;
    }

    int destFd = open(destFile, O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, 0666);
    if (destFd == -1) {
        perror("Error with destination file");
        return 1;
    }

    struct stat fileStat;
    int checkSize = fstat(sourceFd, &fileStat);
    if (checkSize == -1) {
        perror("Error with size of source file");
        return 1;
    }

    int val = fileStat.st_size / COUNT;
    if (fileStat.st_size % COUNT != 0) val++;

    char buffer[COUNT][BUFFER_SIZE];
    struct aio_operation read_aio_op[COUNT];
    struct aio_operation write_aio_op[COUNT];

    for (int i = 0; i < COUNT; i++)
    {
        memset(&read_aio_op[i], 0, sizeof(struct aio_operation));
        read_aio_op[i].aio.aio_fildes = sourceFd;
        read_aio_op[i].aio.aio_buf = buffer[i];
        read_aio_op[i].aio.aio_nbytes = BUFFER_SIZE;
        read_aio_op[i].write_operation = 0;
        read_aio_op[i].aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
        read_aio_op[i].aio.aio_sigevent.sigev_notify_function = aio_completion_handler;
        read_aio_op[i].aio.aio_sigevent.sigev_value.sival_ptr = &read_aio_op[i];
        read_aio_op[i].aio.aio_offset = BUFFER_SIZE * i;
        read_aio_op[i].num = i;

        memset(&write_aio_op[i], 0, sizeof(struct aio_operation));
        write_aio_op[i].aio.aio_fildes = destFd;
        write_aio_op[i].aio.aio_buf = buffer[i];
        write_aio_op[i].aio.aio_nbytes = BUFFER_SIZE;
        write_aio_op[i].aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
        write_aio_op[i].aio.aio_sigevent.sigev_notify_function = aio_completion_handler;
        write_aio_op[i].aio.aio_sigevent.sigev_value.sival_ptr = &write_aio_op[i];
        write_aio_op[i].write_operation = 1;
        write_aio_op[i].aio.aio_offset = BUFFER_SIZE * i;
        write_aio_op[i].num = i;

        read_aio_op[i].next_operation = &write_aio_op[i];
        write_aio_op[i].next_operation = &read_aio_op[i];
    }

    auto begin = chrono::steady_clock::now();

    for (int i = 0; i < COUNT; i++)
    {
        aio_read(&read_aio_op[i].aio);
    }

    while (WriteB != fileStat.st_size)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    cout << "Close files: ";
    close(sourceFd);
    close(destFd);
    cout << "success" << endl;

    auto end = chrono::steady_clock::now();

    auto ms = chrono::duration_cast<chrono::milliseconds>(end - begin);

    cout << "Source file: " << sourceFile << endl;
    cout << "New file: " << destFile << endl;
    cout << "New file size: " << fileStat.st_size << " b" << endl;
    cout << "Write size: " << WriteB << " b" << endl;
    cout << "Time: " << ms.count() << " ms" << endl;
    return 0;
}
