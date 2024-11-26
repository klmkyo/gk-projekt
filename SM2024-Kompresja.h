#pragma once

#include <vector>
#include <utility>
#include <cstdint>
#include <iostream>
#include <cstring> 

struct slowo {
    uint16_t kod = 0;              
    uint8_t dlugosc = 0;           
    uint8_t element[4096] = {0};   
    bool wSlowniku = false;      
};

void wyswietlSlowo(const slowo& aktualneSlowo);

bool porownajSlowa(const slowo& slowo1, const slowo& slowo2);

int znajdzWSlowniku(const std::vector<slowo>& slownik, int rozmiarSlownika, const slowo& szukany);

slowo polaczSlowo(const slowo& aktualneSlowo, uint8_t znak);

slowo noweSlowo();

slowo noweSlowo(uint8_t znak);

int dodajDoSlownika(std::vector<slowo>& slownik, int& rozmiarSlownika, const slowo& nowy, bool czyWyswietlac = true);

void LZWInicjalizacja(std::vector<slowo>& slownik, int& rozmiarSlownika);

std::pair<std::vector<int>, std::vector<slowo>> LZWKompresja(const int wejscie[], int dlugosc);

std::vector<int> LZWDekompresja(const std::vector<int>& skompresowane, const std::vector<slowo>& slownik);