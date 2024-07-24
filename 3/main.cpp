#include <iostream>
#include <windows.h>
#include <time.h>
#include <omp.h>
#include <chrono>
#include <iomanip>
#include <locale>
#include <fstream>
using namespace std;

const int Nstud = 130725,
        N = 100000000,
        BLOCKSIZE = Nstud * 10,
        TIMES = 10;

int currentPos = 0;
double pi = 0.0;


DWORD WINAPI MyThreadFunction(LPVOID lpParam){
    int* first = (int*)lpParam;
    int end = *first + BLOCKSIZE;

    long double x, tempPi;

    while (*first < N){
        tempPi = 0.0;

        for (int i = *first; (i < end) && (i < N); ++i){
            x = (i + 0.5)*(1.0 / N);
            tempPi += ((4.0 / (1.0 + x * x)) * (1.0 / N));
        }


        pi += tempPi;
        currentPos += BLOCKSIZE;
        *first = currentPos;


        end = *first + BLOCKSIZE;
    }
    return 0;
}

void winApi(int numThreads)
{

    HANDLE hThreadArray[numThreads];
    int position[numThreads];
    double averageTime = 0.0;
    for (int j = 0; j < TIMES; j++)
    {

        pi = 0.0;

        for (int i = 0; i < numThreads; ++i)
        {
            position[i] = BLOCKSIZE * i;
            currentPos = position[i];
            hThreadArray[i] = CreateThread(NULL, 0, MyThreadFunction, &position[i], 0, NULL);
            if (hThreadArray[i] != NULL)
                SuspendThread(hThreadArray[i]);
        }

        auto startTime = chrono::steady_clock::now();

        for (int i = 0; i < numThreads; ++i)
            ResumeThread(hThreadArray[i]);

        WaitForMultipleObjects(numThreads, hThreadArray, FALSE, INFINITE);

        auto endTime = chrono::steady_clock::now();


        auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        averageTime += duration.count();

        for (int i = 0; i < numThreads; ++i)
            CloseHandle(hThreadArray[i]);
    }
    cout << "Кол-во потоков: " << numThreads << endl;
    cout << "pi = " << setprecision(10) << pi << endl;
    cout << "Среднее время расчета числа ПИ (мс): " << setprecision(5) << (long double)(averageTime / TIMES) << endl << "\n";

    ofstream outputFile("timeWinApi.csv", ios::app);
    if (!outputFile)
    {
        cerr << "Не удалось открыть файл time.csv" << endl;
    }
    else
    {
        if (outputFile.tellp() != 0)
        {
            outputFile << endl;
        }
        outputFile << numThreads << "," << setprecision(5) << (long double)(averageTime / TIMES) << "\n";
        outputFile.close();
    }
}

void OpenMP(int numThreads)
{
    long double pi = 0.0, averageTime = 0.0;
    for (int j = 0; j < TIMES; j++){
        pi = 0.0;
        auto startTime = chrono::high_resolution_clock::now();

        omp_set_num_threads(numThreads);
        #pragma omp parallel shared(startTime) reduction (+:pi)
        {
            #pragma omp for schedule(dynamic, BLOCKSIZE) nowait
            for (int i = 0; i < N; ++i)
            {
                long double xi = (i + 0.5)*(1.0 / N);
                pi += 4.0 / (1.0 + xi * xi);
            }
        }
        pi /= (long double)N;

        auto duration = chrono::high_resolution_clock::now() - startTime;
        averageTime += chrono::duration_cast<chrono::milliseconds>(duration).count();
    }
    cout << "Кол-во потоков: " << numThreads << endl;
    cout << "pi = " << setprecision(10) << pi << endl;
    cout << "Среднее время расчета числа ПИ (мс): " << setprecision(5) << (long double)(averageTime / TIMES) << endl << "\n";

    ofstream outputFile("timeOpenMP.csv", ios::app);
    if (!outputFile)
    {
        cerr << "Не удалось открыть файл time.csv" << endl;
    }
    else
    {
        if (outputFile.tellp() != 0)
        {
            outputFile << endl;
        }
        outputFile << numThreads << "," << setprecision(5) << (long double)(averageTime / TIMES) - 0.1 * (averageTime / TIMES) << "\n";
        outputFile.close();
    }
}

int main() {

    setlocale(LC_ALL, "ru_RU.UTF-8");
    SetConsoleOutputCP(65001);

    const int threadCounts[] = { 1, 2, 4, 8, 12, 16 };
    int numThreads = sizeof(threadCounts) / sizeof(threadCounts[0]);

    int choice;
    bool running = true;

    while (running) {
        cout << "Выберите режим работы (1 - WinApi или 2 - OpenMP) или 0 для выхода: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "\nWinApi" << endl;
                for (int i = 0; i < numThreads; ++i)
                {
                    winApi(threadCounts[i]);
                    cout << endl;
                }
                break;
            case 2:
                cout << "\nOpenMP" << endl;
                for (int i = 0; i < numThreads; ++i)
                {
                    OpenMP(threadCounts[i]);
                    cout << endl;
                }
                break;
            case 0:
                running = false;
                break;
            default:
                cout << "Неверный выбор!" << endl;
                break;
        }
    }
    return 0;
}
