#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

struct slowo {
  uint16_t kod = 0;
  uint8_t dlugosc = 0;
  uint64_t element[4096];
  bool wSlowniku = false;
};

void wyswietlSlowo(slowo aktualneSlowo) {
  if (aktualneSlowo.wSlowniku) {
    std::cout << "[" << aktualneSlowo.kod << "] ";
  } else {
    std::cout << "[X] ";
  }
  for (int s = 0; s < aktualneSlowo.dlugosc; s++) {
    std::cout << (int)aktualneSlowo.element[s];
    if (s < aktualneSlowo.dlugosc - 1) {
      std::cout << ",";
    }
  }
  std::cout << '\n';
}

bool porownajSlowa(slowo slowo1, slowo slowo2) {
  if (slowo1.dlugosc != slowo2.dlugosc) {
    return false;
  }
  for (int s = 0; s < slowo1.dlugosc; s++) {
    if (slowo1.element[s] != slowo2.element[s]) {
      return false;
    }
  }
  return true;
}

int znajdzWSlowniku(const std::vector<slowo> &slownik, int rozmiarSlownika,
                    slowo szukany) {
  for (int nr = 0; nr < rozmiarSlownika; nr++) {
    if (porownajSlowa(slownik[nr], szukany)) {
      return nr;
    }
  }
  return -1;
}

slowo polaczSlowo(slowo aktualneSlowo, uint8_t znak) {
  slowo noweSlowo;

  if (aktualneSlowo.dlugosc < 4096) {
    noweSlowo.kod = 0;
    noweSlowo.dlugosc = aktualneSlowo.dlugosc + 1;
    noweSlowo.wSlowniku = false;
    std::copy(std::begin(aktualneSlowo.element),
              std::begin(aktualneSlowo.element) + aktualneSlowo.dlugosc,
              std::begin(noweSlowo.element));
    noweSlowo.element[aktualneSlowo.dlugosc] = znak;
    return noweSlowo;
  }
  std::cout << "UWAGA! przepelnienie rozmiaru znakow w pojedynczym slowie"
            << '\n';
  noweSlowo.kod = 0;
  noweSlowo.dlugosc = 0;
  noweSlowo.wSlowniku = false;
  noweSlowo.element[0] = znak;
  return noweSlowo;
}

slowo noweSlowo() {
  slowo noweSlowo;
  noweSlowo.kod = 0;
  noweSlowo.dlugosc = 0;
  noweSlowo.wSlowniku = false;
  return noweSlowo;
}

slowo noweSlowo(uint8_t znak) {
  slowo noweSlowo;
  noweSlowo.kod = 0;
  noweSlowo.dlugosc = 1;
  noweSlowo.element[0] = znak;
  noweSlowo.wSlowniku = false;
  return noweSlowo;
}

int dodajDoSlownika(std::vector<slowo> &slownik, int &rozmiarSlownika,
                    slowo nowy, bool czyWyswietlac = true) {
  if (rozmiarSlownika < 65536) {
    uint16_t nr = rozmiarSlownika;
    slownik[nr].kod = nr;
    slownik[nr].dlugosc = nowy.dlugosc;
    std::copy(std::begin(nowy.element), std::begin(nowy.element) + nowy.dlugosc,
              std::begin(slownik[nr].element));
    slownik[nr].wSlowniku = true;
    if (czyWyswietlac) {
      wyswietlSlowo(slownik[nr]);
    }
    rozmiarSlownika++;
    return nr;
  }
  return -1;
}

void LZWInicjalizacja(std::vector<slowo> &slownik, int &rozmiarSlownika) {
  rozmiarSlownika = 0;

  for (int s = 0; s < 65536; s++) {
    slownik[s].kod = 0;
    slownik[s].dlugosc = 0;
    slownik[s].wSlowniku = false;
    memset(slownik[s].element, 0, sizeof(slownik[s].element));
  }

  slowo noweSlowo;
  for (int s = 0; s < 64; s++) {
    noweSlowo.dlugosc = 1;
    noweSlowo.element[0] = s;
    noweSlowo.kod = dodajDoSlownika(slownik, rozmiarSlownika, noweSlowo);
  }
}

std::pair<std::vector<int>, std::vector<slowo>>
LZWKompresja(const int wejscie[], int dlugosc) {
  std::vector<slowo> slownik(65536);
  int rozmiarSlownika = 0;
  LZWInicjalizacja(slownik, rozmiarSlownika);
  slowo aktualneSlowo = noweSlowo();
  slowo slowoZnak;
  uint8_t znak;
  int kod;
  int i = 0;

  std::vector<int> skompresowane;

  while (i < dlugosc) {
    znak = wejscie[i];
    std::cout << "pobieramy znak " << (int)znak << " z pozycji " << i << '\n';

    slowoZnak = polaczSlowo(aktualneSlowo, znak);
    std::cout << "aktualne slowo: ";
    wyswietlSlowo(aktualneSlowo);
    std::cout << "slowo+znak: ";
    wyswietlSlowo(slowoZnak);

    kod = znajdzWSlowniku(slownik, rozmiarSlownika, slowoZnak);
    std::cout << "czy w slowniku? ";
    if (kod < 0) {
      std::cout << "NIE: [" << kod << "]" << '\n';
      dodajDoSlownika(slownik, rozmiarSlownika, slowoZnak);
      int kodAktualnegoSlowa =
          znajdzWSlowniku(slownik, rozmiarSlownika, aktualneSlowo);
      skompresowane.push_back(kodAktualnegoSlowa);
      std::cout << "dodajemy do skompresowanych: " << kodAktualnegoSlowa
                << '\n';
      aktualneSlowo = noweSlowo(znak);
    } else {
      std::cout << "TAK: [" << kod << "]" << '\n';
      aktualneSlowo = slowoZnak;
    }
    i++;
  }

  int kodAktualnegoSlowa =
      znajdzWSlowniku(slownik, rozmiarSlownika, aktualneSlowo);
  skompresowane.push_back(kodAktualnegoSlowa);
  std::cout << "koniec danych" << '\n';

  return make_pair(skompresowane, slownik);
}

std::vector<int> LZWDekompresja(const std::vector<int> &skompresowane,
                                const std::vector<slowo> &slownik) {
  std::vector<int> zdekompresowane;

  for (auto kod : skompresowane) {
    slowo aktualneSlowo = slownik[kod];
    for (int s = 0; s < aktualneSlowo.dlugosc; s++) {
      zdekompresowane.push_back(aktualneSlowo.element[s]);
    }
  }

  return zdekompresowane;
}