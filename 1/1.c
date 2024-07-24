#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <shlwapi.h>
#include <sys/stat.h>
#include <time.h>

void printDriveList() {
    DWORD drivesMask = GetLogicalDrives();
    printf("Список дисков:\n");

    for (int i = 0; i < 26; i++) {
        if ((drivesMask >> i) & 1) {
            char driveLetter = 'A' + i;
            printf("%c:\\\n", driveLetter);
        }
    }
}

void getDriveType(char* drive) {
    UINT type = GetDriveType(drive);
    printf("Тип диска: ");
    switch (type) {
        case DRIVE_UNKNOWN:
            printf("Неизвестный\n");
            break;
        case DRIVE_NO_ROOT_DIR:
            printf("Нет корневого каталога\n");
            break;
        case DRIVE_REMOVABLE:
            printf("Съемный диск\n");
            break;
        case DRIVE_FIXED:
            printf("Фиксированный диск\n");
            break;
        case DRIVE_REMOTE:
            printf("Удаленный диск\n");
            break;
        case DRIVE_CDROM:
            printf("CD/DVD диск\n");
            break;
        case DRIVE_RAMDISK:
            printf("RAM диск\n");
            break;
        default:
            printf("Неизвестный тип диска\n");
            break;
    }
}

void printFileSysFlags(DWORD dwFileSysFlags) {
    struct FlagDefine {
        DWORD flag;
        const char* description;
    };

    static const struct FlagDefine file_sys_flags[] = {
            { FILE_CASE_SENSITIVE_SEARCH, "Case sensitive search" },
            { FILE_CASE_PRESERVED_NAMES, "Case preserved names" },
            { FILE_UNICODE_ON_DISK, "Unicode on disk" },
            { FILE_PERSISTENT_ACLS, "Persistent ACLs" },
            { FILE_FILE_COMPRESSION, "File compression" },
            { FILE_VOLUME_QUOTAS, "Volume quotas" },
            { FILE_SUPPORTS_SPARSE_FILES, "Supports sparse files" },
            { FILE_SUPPORTS_REPARSE_POINTS, "Supports reparse points" },
            { FILE_SUPPORTS_REMOTE_STORAGE, "Supports remote storage" },
            { FILE_VOLUME_IS_COMPRESSED, "Volume is compressed" },
            { FILE_SUPPORTS_OBJECT_IDS, "Supports object IDs" },
            { FILE_SUPPORTS_ENCRYPTION, "Supports encryption" },
            { FILE_NAMED_STREAMS, "Named streams" },
            { FILE_READ_ONLY_VOLUME, "Read-only volume" },
            { FILE_SEQUENTIAL_WRITE_ONCE, "Sequential write once" },
            { FILE_SUPPORTS_TRANSACTIONS, "Supports transactions" },
    };

    printf("Флаги файловой системы:\n");

    for (int i = 0; i < sizeof(file_sys_flags) / sizeof(struct FlagDefine); i++) {
        if (dwFileSysFlags & file_sys_flags[i].flag) {
            printf("  - %s\n", file_sys_flags[i].description);
        }
    }
}

void getDriveCommonInfo(char* drive) {
    char szVolName[MAX_PATH + 1], szFileSysName[MAX_PATH + 1];
    DWORD dwSerialNumber, dwMaxComponentLen, dwFileSysFlags;
    GetVolumeInformation(drive, szVolName, MAX_PATH,
                         &dwSerialNumber, &dwMaxComponentLen,
                         &dwFileSysFlags, szFileSysName, sizeof(szFileSysName));
    printf("Серийный номер: %lu\n", dwSerialNumber);
    printf("Длина максимального компонента: %lu\n", dwMaxComponentLen);
    printFileSysFlags(dwFileSysFlags);
}

void getDriveFreeSpace(char* drive) {
    DWORD sectorsPerCluster;
    DWORD bytesPerSector;
    DWORD freeClusters;
    DWORD totalClusters;

    GetDiskFreeSpace(
            drive,
            &sectorsPerCluster,
            &bytesPerSector,
            &freeClusters,
            &totalClusters
    );

    unsigned int kBPerCluster = bytesPerSector * sectorsPerCluster / 1024;

    printf("Свободное место: %lu MB\n", kBPerCluster * freeClusters / 1024);
}

void printDiskInfo(char driveLetter) {
    char rootPath[4];
    sprintf(rootPath, "%c:\\", driveLetter);

    printf("\nИнформация о диске %c:\n", driveLetter);

    getDriveType(rootPath);
    getDriveCommonInfo(rootPath);
    getDriveFreeSpace(rootPath);
}

void createDirectory() {
    printf("\nВведите путь к каталогу, который хотите создать: ");
    char directoryPath[MAX_PATH];
    scanf("%s", directoryPath);
    if (CreateDirectory(directoryPath, NULL)) {
        printf("Каталог успешно создан\n");
    } else {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_ALREADY_EXISTS) {
            printf("Каталог уже существует\n");
        } else {
            printf("Произошла ошибка при создании каталога. Код ошибки: %lu\n", errorCode);
        }
    }
}

void removeDirectory() {
    printf("\nВведите путь к каталогу, который хотите удалить: ");
    char directoryPath[MAX_PATH];
    scanf("%s", directoryPath);

    if (RemoveDirectory(directoryPath)) {
        printf("Каталог успешно удален\n");
    } else {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_DIR_NOT_EMPTY) {
            printf("Каталог не пустой. Удаление невозможно\n");
        } else if (errorCode == ERROR_PATH_NOT_FOUND) {
            printf("Указанный каталог не найден\n");
        } else {
            printf("Произошла ошибка при удалении каталога. Код ошибки: %lu\n", errorCode);
        }
    }
}

void createFile() {
    printf("\nВведите путь к файлу, который хотите создать: ");
    char filePath[MAX_PATH];
    scanf("%s", filePath);
    HANDLE fileHandle = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        printf("Файл успешно создан\n");
        CloseHandle(fileHandle);
    } else {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_FILE_EXISTS) {
            printf("Файл уже существует\n");
        } else {
            printf("Произошла ошибка при создании файла. Код ошибки: %lu\n", errorCode);
        }
    }
}

void copyMoveFile(BOOL isMoveOperation) {
    printf("\nВведите путь к исходному файлу: ");
    char sourcePath[MAX_PATH];
    scanf("%s", sourcePath);

    printf("Введите путь к целевому каталогу: ");
    char targetDirectory[MAX_PATH];
    scanf("%s", targetDirectory);


    char targetPath[MAX_PATH];
    snprintf(targetPath, sizeof(targetPath), "%s\\%s", targetDirectory, PathFindFileName(sourcePath));

    if (isMoveOperation) {
        if (MoveFile(sourcePath, targetPath)) {
            printf("Файл успешно перемещен\n");
        } else {
            DWORD errorCode = GetLastError();
            printf("Произошла ошибка при перемещении файла. Код ошибки: %lu\n", errorCode);
        }
    } else {
        if (CopyFile(sourcePath, targetPath, FALSE)) {
            printf("Файл успешно скопирован\n");
        } else {
            DWORD errorCode = GetLastError();
            printf("Произошла ошибка при копировании файла. Код ошибки: %lu\n", errorCode);
        }
    }
}

void analyzeAndModifyFileAttributes() {
    printf("\nВведите путь к файлу: ");
    char filePath[MAX_PATH];
    scanf("%s", filePath);

    DWORD fileAttributes = GetFileAttributes(filePath);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        DWORD errorCode = GetLastError();
        printf("Произошла ошибка при получении атрибутов файла. Код ошибки: %lu\n", errorCode);
        return;
    }

    printf("Атрибуты файла:\n");

    if (fileAttributes & FILE_ATTRIBUTE_READONLY) {
        printf("- Только для чтения\n");
    }
    if (fileAttributes & FILE_ATTRIBUTE_HIDDEN) {
        printf("- Скрытый\n");
    }
    if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        printf("- Каталог\n");
    }
    if (fileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
        printf("- Архивный\n");
    }
    if (fileAttributes & FILE_ATTRIBUTE_NORMAL) {
        printf("- Обычный\n");
    }


    printf("\nИзменить атрибуты файла (1 - Да, 0 - Нет): ");
    int modifyAttributes;
    scanf("%d", &modifyAttributes);

    if (modifyAttributes) {
        DWORD newFileAttributes = fileAttributes;

        printf("\n1 - Архивный\n"
               "2 - Только для чтения\n"
               "3 - Скрытый\n"
               "4 - Обычный\n"

               "Выберите новый атрибут файла (введите номер): ");
        int attributeSelection;
        scanf("%d", &attributeSelection);

        switch (attributeSelection) {
            case 1:
                if (fileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
                    newFileAttributes &= ~FILE_ATTRIBUTE_ARCHIVE;
                } else {
                    newFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                }
                break;
            case 2:
                if (fileAttributes & FILE_ATTRIBUTE_READONLY)
                    newFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
                else
                    newFileAttributes |= FILE_ATTRIBUTE_READONLY;
                break;
            case 3:
                if (fileAttributes & FILE_ATTRIBUTE_HIDDEN)
                    newFileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
                else
                    newFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                break;
            case 4:
                newFileAttributes = FILE_ATTRIBUTE_NORMAL;
                break;
            default:
                printf("Неправильный выбор. Атрибуты не будут изменены\n");
                return;
        }

        if (SetFileAttributes(filePath, newFileAttributes)) {
            printf("Атрибуты файла успешно изменены\n");
        } else {
            DWORD errorCode = GetLastError();
            printf("Произошла ошибка при изменении атрибутов файла. Код ошибки: %lu\n", errorCode);
        }
    }
}

void printFileCreationTime(const char* path) {
    struct stat attrib;
    if (stat(path, &attrib) == 0) {
        struct tm* creationTime = (struct tm *) localtime(&attrib.st_ctime);
        char timeString[100];
        strftime(timeString, sizeof(timeString), "%c", creationTime);
        printf("Время создания файла: %s\n", timeString);
    } else {
        printf("Ошибка при получении информации о файле\n");
    }
}

void printFileLastAccessTime(const char* path) {
    struct stat attrib;
    if (stat(path, &attrib) == 0) {
        struct tm* LastAccessTime = (struct tm *) localtime(&attrib.st_atime);
        char timeString[100];
        strftime(timeString, sizeof(timeString), "%c", LastAccessTime);
        printf("Время последнего доступа к файлу: %s\n", timeString);
    } else {
        printf("Ошибка при получении информации о файле\n");
    }
}

void printFileLastModifyTime(const char* path) {
    struct stat attrib;
    if (stat(path, &attrib) == 0) {
        struct tm* LastModifyTime = (struct tm *) localtime(&attrib.st_mtime);
        char timeString[100];
        strftime(timeString, sizeof(timeString), "%c", LastModifyTime);
        printf("Время последнего изменения файла: %s\n", timeString);
    } else {
        printf("Ошибка при получении информации о файле\n");
    }
}

void setTime() {

    printf("\nВведите путь к файлу: ");
    char path[MAX_PATH];
    scanf("%s", path);

    if (access(path, F_OK) != 0) {
        printf("Ошибка: неправильный путь к файлу\n");
        return;
    }

    int choiceTime = 0;
    printf("Выберите, какое время хотите изменить.\n1 - создания\n2 - последнего доступа\n3 - последнего изменения\n");
    scanf("%d", &choiceTime);

    if (choiceTime == 1) {
        SYSTEMTIME sysCreationTime = { 0 };
        printf("Время создания\n");
        printFileCreationTime(path);

        printf("\tВведите день: ");
        scanf("%hu", &sysCreationTime.wDay);
        printf("\tВведите месяц: ");
        scanf("%hu", &sysCreationTime.wMonth);
        printf("\tВведите год: ");
        scanf("%hu", &sysCreationTime.wYear);

        FILETIME fileCreationTime;
        SystemTimeToFileTime(&sysCreationTime, &fileCreationTime);

        HANDLE handlerFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handlerFile != INVALID_HANDLE_VALUE) {
            if (SetFileTime(handlerFile, &fileCreationTime, NULL, NULL)) {
                printf("Дата успешно изменена\n");
                printFileCreationTime(path);
            } else {
                DWORD err = GetLastError();
                printf("Ошибка при изменении информации времени\n%lu\n", err);
            }
            CloseHandle(handlerFile);
        }
        choiceTime = 0;
    }

    if (choiceTime == 2) {
        SYSTEMTIME sysTime = { 0 };
        printf("Дата последнего доступа\n");
        printFileLastAccessTime(path);
        printf("\tВведите день: ");
        scanf("%hu", &sysTime.wDay);
        printf("\tВведите месяц: ");
        scanf("%hu", &sysTime.wMonth);
        printf("\tВведите год: ");
        scanf("%hu", &sysTime.wYear);
        FILETIME fileTime;
        SystemTimeToFileTime(&sysTime, &fileTime);
        HANDLE handlerFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handlerFile != INVALID_HANDLE_VALUE) {
            if (SetFileTime(handlerFile, NULL, &fileTime, NULL)) {
                printf("Дата успешно изменена\n");
            } else {
                DWORD err = GetLastError();
                printf("Ошибка при изменении информации времени\n%lu\n", err);
            }
            CloseHandle(handlerFile);
        }
        choiceTime = 0;
    }

    if (choiceTime == 3) {
        SYSTEMTIME sysTime = { 0 };
        printf("Дата последнего изменения\n");

        printFileLastModifyTime(path);

        printf("\tВведите день: ");
        scanf("%hu", &sysTime.wDay);

        printf("\tВведите месяц: ");
        scanf("%hu", &sysTime.wMonth);

        printf("\tВведите год: ");
        scanf("%hu", &sysTime.wYear);

        FILETIME fileTime;
        SystemTimeToFileTime(&sysTime, &fileTime);

        HANDLE handlerFile = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handlerFile != INVALID_HANDLE_VALUE) {
            if (SetFileTime(handlerFile, NULL, NULL, &fileTime)) {
                printf("Дата успешно изменена\n");
            } else {
                DWORD err = GetLastError();
                printf("Ошибка при изменении информации времени\n%lu\n", err);
            }
            CloseHandle(handlerFile);
        }

        choiceTime = 0;
    }
}

int main() {
    setlocale(LC_ALL, "Rus");
    int choice;

    do {
        printf("\nМеню:\n");
        printf("1. Вывод списка дисков\n");
        printf("2. Вывод информации о диске и размере свободного пространства\n");
        printf("3. Создание каталога\n");
        printf("4. Удаление каталога\n");
        printf("5. Создание файла\n");
        printf("6. Копирование файла\n");
        printf("7. Перемещение файла\n");
        printf("8. Анализ и изменение атрибутов файла\n");
        printf("9. Изменение времени файла\n");
        printf("0. Выход\n");
        printf("Выберите действие: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printDriveList();
                break;
            case 2:
                printf("\nВведите букву диска: ");
                char driveLetter;
                scanf(" %c", &driveLetter);
                printDiskInfo(driveLetter);
                break;
            case 3:
                createDirectory();
                break;
            case 4:
                removeDirectory();
                break;
            case 5:
                createFile();
                break;
            case 6:
                copyMoveFile(FALSE);
                break;
            case 7:
                copyMoveFile(TRUE);
                break;
            case 8:
                analyzeAndModifyFileAttributes();
                break;
            case 9:
                setTime();
                break;
            case 0:
                printf("\nПрограмма завершена\n");
                break;
            default:
                printf("\nНеправильный выбор. Попробуйте еще раз\n");
                break;
        }
    } while (choice != 0);

    return 0;
}
