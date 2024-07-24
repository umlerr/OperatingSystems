#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <arpa/inet.h> 
#include <fcntl.h>


#define FILENAME "file.txt"

int client_send_status() {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 
    char sendBuff[1025];
    time_t ticks; 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(listenfd, 10); 
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    snprintf(sendBuff, sizeof(sendBuff), "readed");
    write(connfd, sendBuff, strlen(sendBuff)); 
    close(connfd);
}

int main() {
    int choice;
    int result;
    int secret_file;
    char *ptr;
    fd_set read_fds;
    struct timeval timeout;
    struct stat statbuf;
    do {
        printf("\nМеню клиента:\n");
        printf("1. Проецирование\n");
        printf("2. Считывание\n");
        printf("3. Выход\n");
        printf("Выбор: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                secret_file = open(FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                fstat(secret_file, &statbuf);
                ptr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, secret_file, 0);
                if(ptr == MAP_FAILED) printf("Проецирование неудачно\n");
                else printf("Проецирование удачно\n");
                break;
            case 2:
                timeout.tv_sec = 20;
                FD_ZERO(&read_fds);
                FD_SET(secret_file, &read_fds);
                result = select(secret_file + 1, &read_fds, NULL, NULL, &timeout);
                if (result == -1) {
                    printf("Ошибка выбора\n");
                }
                else if (result == 0 ) {
                    printf("Время выполнения превышено\n");
                }
                else {
                    printf("Клиент получил: %s\n", ptr);
                }
                printf("Сигнал к серверу\n");
                client_send_status();
                break;
            case 3:
                munmap(ptr, statbuf.st_size);
                choice = 0;
                break;
            default:
                printf("\n Неверное значение\n");
                break;
        }
    } while (choice != 0);
    return 0;
}