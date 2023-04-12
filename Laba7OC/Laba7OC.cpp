#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <synchapi.h>

class Stack {
private:
    int* array;
    int size;
    int topIndex;
public:
    Stack() {
        size = 10;
        array = new int[size];
        topIndex = -1;
    }

    ~Stack() {
        delete[] array;
    }

    // Метод для добавления элемента в стек
    void push(int element) {
        if (topIndex == size - 1) {
            int* newArray = new int[size * 2];
            for (int i = 0; i < size; i++) {
                newArray[i] = array[i];
            }
            delete[] array;
            array = newArray;
            size *= 2;
        }
        topIndex++;
        array[topIndex] = element;
        saveToFile("myStack.bin"); // Сохраняем измененный стек на диск
    }

    // Метод для извлечения элемента из стека
    int pop() {
        if (topIndex == -1) {
            std::cerr << "Stack is empty." << std::endl;
            return -1;
        }
        int element = array[topIndex];
        topIndex--;
        saveToFile("myStack.bin"); // Сохраняем измененный стек на диск
        return element;
    }

    int peek() {
        if (topIndex == -1) {
            std::cerr << "Stack is empty." << std::endl;
            return -1;
        }
        return array[topIndex];
    }

    bool isEmpty() {
        return (topIndex == -1);
    }

    void loadFromFile(std::string filename) {
        HANDLE fileHandle = CreateFile(L"filename", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        DWORD bytesRead;
        delete[] array;
        ReadFile(fileHandle, &size, sizeof(int), &bytesRead, NULL);
        array = new int[size];
        ReadFile(fileHandle, &topIndex, sizeof(int), &bytesRead, NULL);
        ReadFile(fileHandle, array, sizeof(int) * (topIndex + 1), &bytesRead, NULL);
        CloseHandle(fileHandle);
    }

    void saveToFile(std::string filename) {
        HANDLE fileHandle = CreateFile(L"filename", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Error creating file." << std::endl;
            return;
        }
        DWORD bytesWritten;
        WriteFile(fileHandle, &size, sizeof(int), &bytesWritten, NULL);
        WriteFile(fileHandle, &topIndex, sizeof(int), &bytesWritten, NULL);
        WriteFile(fileHandle, array, sizeof(int) * (topIndex + 1), &bytesWritten, NULL);
        CloseHandle(fileHandle);
    }
};

int buffer = 0;
DWORD id;
HANDLE emptybuffer;
HANDLE fullbuffer;
HANDLE Check;
HANDLE hThreadPush; // Дескриптор потока для извлечения элементов из стека


// Функция-обработчик для потока, который будет извлекать элементы из стека
DWORD WINAPI pushThread(PVOID lpParam) {
    Stack* myStack = (Stack*)lpParam;
    std::cout << "Thread push started." << std::endl;

    SetEvent(emptybuffer);

    while (WaitForSingleObject(Check, 0) != WAIT_OBJECT_0) {
        WaitForSingleObject(fullbuffer, INFINITE);
        if (WaitForSingleObject(Check, 0) == WAIT_OBJECT_0)
        { 
            break;
        }
        myStack->push(buffer);
        std::cout << buffer << std::endl;
        SetEvent(emptybuffer);
       
    }
    return 0;
    
}

int main() {
    Stack* myStack = new Stack();

    emptybuffer = CreateEventA(NULL, FALSE, FALSE, NULL);
    fullbuffer = CreateEvent(NULL, FALSE, FALSE, NULL);
    Check = CreateEvent(NULL, TRUE, FALSE, NULL);
    // Создаем поток, который будет извлекать элементы из стека
    hThreadPush = CreateThread(NULL, 0, &pushThread, myStack, 0, &id);

    WaitForSingleObject(emptybuffer, INFINITE);
    buffer = 1;
    SetEvent(fullbuffer);

    WaitForSingleObject(emptybuffer, INFINITE);
    buffer = 2;
    SetEvent(fullbuffer);
    
    WaitForSingleObject(emptybuffer, INFINITE);
    buffer = 3;
    SetEvent(fullbuffer);

    WaitForSingleObject(emptybuffer, INFINITE);
    SetEvent(Check);
    SetEvent(fullbuffer);
    WaitForSingleObject(hThreadPush, INFINITE);
    return 0;
}