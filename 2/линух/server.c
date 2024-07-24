#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

#define FILENAME "file.txt"

int client_read_status() {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    memset(recvBuff, '0', sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Ошибка - невозможно создать сокет \n");
        return 2;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n Ошибка - inet_pton error occured\n");
        return 2;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Ошибка подключения\n");
       return 2;
    } 

    while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
        if(strcmp("readed", recvBuff) == 0) return 1;
        printf("Буффер - %s\n", recvBuff); 
    } 

    if(n < 0) printf("\n Ошибка чтения \n");
}

int main() {
    int choice;
    int result;
    int size = 4096;
    int client_status;
    do {
        printf("\nМеню сервера:\n");
        printf("1. Проецирование\n");
        printf("2. Запись\n");
        printf("3. Выход\n");
        printf("Выбор: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                int file = open(FILENAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                int alloc = fallocate(file, 0, 0, size);
                if(alloc == -1) printf("Ошибка резервации памяти файла\n");
                else printf("Резервация памяти файла успешна\n");
                char *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
                if(ptr == MAP_FAILED) printf("Ошибка создания проецируемого файла\n");
                else printf("Создание проецируемого файла успешно\n");
                break;
            case 2:
                char *message = "Message to client !";
                size = strlen(message);
                for (int i = 0; i < size; i++) 
                {
                    printf("Вывод символа %c в %d\n", message[i], i);
                    ptr[i] = message[i];
                }
                break;
            case 3:
                client_status = client_read_status();
                if(client_status == 1) printf("Клиент прочитал сообщение\n");
                else if(client_status == 2) printf("Ошибка сокета\n");
                else printf("Ошибка кода %d", client_status);
                munmap(ptr, size);
                unlink(FILENAME);
                choice = 0;
                break;
            default:
                printf("\n Неверное значение\n");
                break;
        }
    } while (choice != 0);
    return 0;
}