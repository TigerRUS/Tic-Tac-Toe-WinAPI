#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

struct Settings {
    COLORREF BGColor = RGB(0, 0, 255);
    COLORREF gridColor = RGB(255, 0, 0);
    COLORREF objectColor = RGB(255, 255, 255);

    int fieldSize = 3;
    int* arr = nullptr;

    int width = 320;
    int height = 240;
};

void ReadConfigFromFile(int num, Settings *settings);

void WriteConfigToFile(int num, Settings *settings);