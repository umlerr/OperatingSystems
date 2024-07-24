#include <iostream>
#include <windows.h>
#include <bitset>
#include <cstring>
#define CLS system("cls")
using namespace std;
void GetSystemInfoFunc(SYSTEM_INFO&);
void GlobalMemoryStatusFunc();
void AllocatedMemoryStatusFunc();
void VirtualReserveFunc(SYSTEM_INFO&);
void ReservetionWithCommitFunc(SYSTEM_INFO&);
void WriteMemoryFunc();
void SetVirtualProtectionFunc();
void VirtualFreeFunc();

void menu(int& option);
int safe_cin();
void choose_protection(DWORD&);
void protect_info(const DWORD&);

int main() {
    int option = -1;
    SYSTEM_INFO SYSTEM_INFO;
    GetSystemInfo(&SYSTEM_INFO);
    do {
        menu(option);
        switch (option) {
        case 0:
            break;
        case 1:
            GetSystemInfoFunc(SYSTEM_INFO);
            break;
        case 2:
            GlobalMemoryStatusFunc();
            break;
        case 3:
            AllocatedMemoryStatusFunc();
            break;
        case 4:
            VirtualReserveFunc(SYSTEM_INFO);
            break;
        case 5:
            ReservetionWithCommitFunc(SYSTEM_INFO);
            break;
        case 6:
            WriteMemoryFunc();
            break;
        case 7:
            SetVirtualProtectionFunc();
            break;
        case 8:
            VirtualFreeFunc();
            break;
        }
    } while (option);
    cout << "Goodbye" << endl;

    return 0;
}

void menu(int& option) {
    option = -1;
    while (option < 0 || option > 8) {
        cout << "Menu\n";
        cout << "1. Get System Info\n";
        cout << "2. Global Memory Status\n";
        cout << "3. Memory Status From keyboard\n";
        cout << "4. Virtual Alloc (Reserve Only)\n";
        cout << "5. Virtual Alloc and Commit\n";
        cout << "6. Write Data to Memory\n";
        cout << "7. Virtual Protect\n";
        cout << "8. Virtual Free\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        option = safe_cin();
        CLS;
        if (option < 0 || option > 8) cout << "Wrong option\n";
    }

}
int safe_cin() {
    int choice = -1;
    char str[50];
    cin >> str;
    while (sscanf(str, "%d", &choice) != 1) {
        printf("Incorrect input! Try again use only numbers: ");
        cin >> str;
    }
    return choice;
}
void GetSystemInfoFunc(SYSTEM_INFO& SYSTEM_INFO) {
    WORD archType;

    archType = SYSTEM_INFO.wProcessorArchitecture;
    if (archType == PROCESSOR_ARCHITECTURE_AMD64)
        cout << "x64 ";
    else if (archType == PROCESSOR_ARCHITECTURE_ARM)
        cout << "ARM";
    else if (archType == PROCESSOR_ARCHITECTURE_IA64)
        cout << "Intel Itanium-based";
    else if (archType == PROCESSOR_ARCHITECTURE_INTEL)
        cout << "x86";
    else if (archType == PROCESSOR_ARCHITECTURE_UNKNOWN)
        cout << "Unknown";
    cout << " architechture\n";

    cout << "Page size: " << SYSTEM_INFO.dwPageSize << endl;
    cout << "Minimum memory address: " << SYSTEM_INFO.lpMinimumApplicationAddress << endl;
    cout << "Maximum memory address: " << SYSTEM_INFO.lpMaximumApplicationAddress << endl;
    //cout << "A mask representing the set of processors\n"
      //  "configured into the system: " << endl << std::bitset<32>(SYSTEM_INFO.dwActiveProcessorMask) << endl;
    cout << "Logical Processors: " << SYSTEM_INFO.dwNumberOfProcessors << endl;
    cout << "The granularity for the starting address \nat which virtual memory can be allocated: "
        << SYSTEM_INFO.dwAllocationGranularity << endl;
    cout << "The architecture-dependent processor level: " << SYSTEM_INFO.wProcessorLevel << endl;
    cout << "The architecture-dependent processor revision: " << SYSTEM_INFO.wProcessorRevision << "\n\n";
    system("pause");
    CLS;
}
void GlobalMemoryStatusFunc() {
    MEMORYSTATUS MEMORYSTATUS;
    GlobalMemoryStatus(&MEMORYSTATUS);
    cout << "Size of MEMORYSTATUS, in bytes: " << MEMORYSTATUS.dwLength << endl;
    cout << "Percentage of used memory: " << MEMORYSTATUS.dwMemoryLoad << endl;
    cout << "Memory on disk, in bytes: " << MEMORYSTATUS.dwTotalPhys << endl;
    cout << "Available memory on disk, in bytes: " << MEMORYSTATUS.dwAvailPhys << endl;
    cout << "Memory Limit, in bytes: " << MEMORYSTATUS.dwTotalPageFile << endl;
    cout << "The maximum amount of memory the current process can commit, in bytes: " << MEMORYSTATUS.dwAvailPageFile << endl;
    cout << "The size of the user-mode portion of the virtual address space \nof the calling process, in bytes: " << MEMORYSTATUS.dwTotalVirtual << endl;
    cout << "Unreserved and uncommited memory, in bytes: " << MEMORYSTATUS.dwAvailVirtual << endl;
    system("pause");
    CLS;
}
void AllocatedMemoryStatusFunc() {
    LPVOID address;
    MEMORY_BASIC_INFORMATION info;
    DWORD state, type;

    cout << "Enter the required valid address: 0x";
    cin >> std::hex >> address;

    if (VirtualQuery(address, &info, sizeof(info)) == 0) {
        cout << "Error 0x" << GetLastError() << "\n";
        system("pause");
        CLS;
        return;
    }

    cout << "A pointer to the base address of the region of pages to be queried: " << address << endl;
    cout << "A pointer to the base address of the region of pages: " << info.BaseAddress << endl;
    cout << "A pointer to the base address of a range of pages allocated by the VirtualAlloc function: " << info.AllocationBase << endl;
    cout << "The memory protection option when the region was initially allocated: " <<
        info.AllocationProtect << ((info.AllocationProtect != 0) ? "" : " (caller does not have access)") << endl;
    cout << "The size of the region beginning at the base address \n"
        "in which all pages have identical attributes, in bytes: " << info.RegionSize << endl;
    state = info.State;
    if (state == MEM_COMMIT)
        cout << "MEM COMMIT\n";
    else if (state == MEM_FREE)
        cout << "MEM FREE\n";
    else if (state == MEM_RESERVE)
        cout << "MEM RESERVE\n";
    else cout << "Unknown state\n";

    cout << "Protection: " << info.Protect << endl;
    //именно здесь нужна info_protect(так должна быть какая-нибудь константа)
    protect_info(info.Protect);
    cout << endl;
    cout << "The type of pages in the region\n";
    type = info.Type;
    if (type == MEM_IMAGE)
        cout << "MEM IMAGE\n";
    else if (type == MEM_MAPPED)
        cout << "MEM MAPPED\n";
    else if (type == MEM_PRIVATE)
        cout << "MEM PRIVATE\n";
    else cout << "Unknown type\n";

    system("pause");
    CLS;
}

//this will with commit
void VirtualReserveFunc(SYSTEM_INFO& SYSTEM_INFO) {
    int option = -1;
    void* address = nullptr;
    while (option < 1 || option > 2) {
        cout << "What do you wanna choose?\n";
        cout << "1 - Auto\n";
        cout << "2 - Manual\n";
        cout << "Enter your choice: ";
        option = safe_cin();
        CLS;
        if (option < 1 || option > 2) cout << "Wrong option\n";
    }
    if (option == 2) {
        cout << "Enter the required valid address: 0x";
        cin >> address;
    }

    address = VirtualAlloc(address, SYSTEM_INFO.dwPageSize, MEM_RESERVE, PAGE_READWRITE);
    if (address) {
        if (address = VirtualAlloc(address, SYSTEM_INFO.dwPageSize, MEM_COMMIT, PAGE_READWRITE))
            cout << "Memory area allocated\nAddress: " << address << endl;
        else cout << "Error 0x" << GetLastError() << endl << "Memory hasn't commited";
    }
    else cout << "Error 0x" << GetLastError() << endl << "Memory hasn't reserved\n";
}
//and the next function will be with reserve and commit
void ReservetionWithCommitFunc(SYSTEM_INFO& SYSTEM_INFO) {
    int option = -1;
    void* address = nullptr;
    while (option < 1 || option > 2) {
        cout << "What do you wanna choose?\n";
        cout << "1 - Auto\n";
        cout << "2 - Manual\n";
        cout << "Enter your choice: ";
        option = safe_cin();
        CLS;
        if (option < 1 || option > 2) cout << "Wrong option\n";
    }
    if (option == 2) {
        cout << "Enter the required valid address: 0x";
        cin >> address;
    }
    if (address = VirtualAlloc(address, SYSTEM_INFO.dwPageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))
        cout << "Memory area allocated\nAddress: " << address << endl;
    else cout << "Error 0x" << GetLastError() << endl << "Memory hasn't allocated\n";
}
//use CopyMemory
    void WriteMemoryFunc() {
       LPVOID address = nullptr;
       string data;
       std::cout << "Enter the memory address: 0x";
       cin >> std::hex >> address;
       std::cout << "Enter data:";
       cin >> data;
       if (CopyMemory(address, data.c_str(), data.length() * sizeof(char)))  cout << "Data recorded " << endl;
       else  cout << "No memory has been allocated. Error: " << GetLastError() << endl;
    }

void SetVirtualProtectionFunc() {
    LPVOID address;
    DWORD newLevel, oldLevel;

    cout << "Enter the address: 0x";
    cin >> address;
    if (!address) {
        cout << "Address is null\n";
        system("pause");
        return;
    }
    cout << "Choose new protection level:\n";
    choose_protection(newLevel);

    if (VirtualProtect(address, sizeof(DWORD), newLevel, &oldLevel)) {
        cout << "Old protection level:\n";
        protect_info(oldLevel);
    }
    else cout << "Error 0x" << GetLastError() << "\nAccess denied\n";
    system("pause");
}

void choose_protection(DWORD& newLevel) {
    newLevel = -1;
    while (newLevel < 1 || newLevel > 10) {
        //cout << "Enter the memory protection:\n";
        cout << "1 - PAGE_EXECUTE\n";
        cout << "2 - PAGE_EXECUTE_READ\n";
        cout << "3 - PAGE_EXECUTE_READWRITE\n";
        cout << "4 - PAGE_EXECUTE_WRITECOPY\n";
        cout << "5 - PAGE_NOACCESS\n";
        cout << "6 - PAGE_READONLY\n";
        cout << "7 - PAGE_READWRITE\n";
        cout << "8 - PAGE_WRITECOPY\n";
        cout << "9 - PAGE_TARGETS_INVALID\n";
        cout << "10- PAGE_TARGETS_NO_UPDATE\n";
        cout << "Enter your choice: ";
        newLevel = safe_cin();
        CLS;
        if (newLevel < 1 || newLevel > 10) cout << "Wrong option\n";
    }

    if (newLevel == 1) newLevel = PAGE_EXECUTE;
    else if (newLevel == 2) newLevel = PAGE_EXECUTE_READ;
    else if (newLevel == 3) newLevel = PAGE_EXECUTE_READWRITE;
    else if (newLevel == 4) newLevel = PAGE_EXECUTE_WRITECOPY;
    else if (newLevel == 5) newLevel = PAGE_NOACCESS;
    else if (newLevel == 6) newLevel = PAGE_READONLY;
    else if (newLevel == 7) newLevel = PAGE_READWRITE;
    else if (newLevel == 8) newLevel = PAGE_WRITECOPY;
    else if (newLevel == 9) newLevel = PAGE_TARGETS_INVALID;
    else newLevel = PAGE_TARGETS_NO_UPDATE;
}

void protect_info(const DWORD& pro) {
    if (pro & PAGE_EXECUTE) cout << "PAGE_EXECUTE\n";
    if (pro & PAGE_EXECUTE_READ) cout << "PAGE_EXECUTE_READ\n";
    if (pro & PAGE_EXECUTE_READWRITE) cout << "PAGE_EXECUTE_READWRITE\n";
    if (pro & PAGE_EXECUTE_WRITECOPY) cout << "PAGE_EXECUTE_WRITECOPY\n";
    if (pro & PAGE_NOACCESS) cout << "PAGE_NOACCESS\n";
    if (pro & PAGE_READONLY) cout << "PAGE_READONLY\n";
    if (pro & PAGE_READWRITE) cout << "PAGE_READWRITE\n";
    if (pro & PAGE_WRITECOPY) cout << "PAGE_WRITECOPY\n";
    if (pro & PAGE_TARGETS_INVALID) cout << "PAGE_TARGETS_INVALID\n";
    if (pro & PAGE_TARGETS_NO_UPDATE) cout << "PAGE_TARGETS_NO_UPDATE\n";
}

void VirtualFreeFunc() {
    LPVOID address = NULL;
    cout << "Enter the address: 0x";
    cin >> address;

    if (VirtualFree(address, 0, MEM_RELEASE))
        cout << "Memory area's released\n";
    else std::cerr << "Error: 0x" << GetLastError() << endl;
    system("pause");
}
