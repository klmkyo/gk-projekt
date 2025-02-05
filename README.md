# 🐟 North Fish Format (NF) - Jak Rust, ale Bardziej Rybny! 🐟

Czy masz dość zawiłych formatów obrazów? Spróbuj czegoś świeżego prosto z morza! North Fish Format (NF) to nowoczesny format kompresji obrazów, inspirowany świeżością dorsza i bezpieczeństwem Rusta!

**Bezpieczeństwo Pamięci:**

- Tak jak Rust dba o bezpieczeństwo pamięci, tak North Fish dba o świeżość Twoich obrazów
- Zero wycieków pamięci - jedyny wyciek to sos tatarski!

**Współbieżność bez Bólu Głowy:**

- Rust eliminuje wyścigi, a North Fish eliminuje wyścigi do ostatniego kawałka łososia
- Przetwarzaj obrazy równolegle jak sprawny kucharz North Fish przygotowujący zamówienia

**Zero Garbage Collection:**

- Rust nie potrzebuje garbage collectora
- North Fish nie zostawia żadnych resztek - wszystko jest świeże i wykorzystane!

## 🎣 Jak używać?

```bash
# Kompilacja (Ubuntu/Debian):
clang++ -o SM2024-Projekt.out SM2024-Projekt.cpp SM2024-Pliki.cpp SM2024-Zmienne.cpp -O2 -std=c++17 $(pkg-config --cflags --libs sdl2)

# Konwersja z BMP do NF (jak zamawianie świeżej ryby):
./SM2024-Projekt.out convert -i input.bmp -o output.nf -t rgb555 -f none -c none

# Konwersja z NF do BMP (jak podawanie ryby na talerz):
./SM2024-Projekt.out convert -i input.nf -o output.bmp

# Podgląd obrazu NF (jak oglądanie wystawy w North Fish):
./SM2024-Projekt.out view -i image.nf

# Informacje o pliku NF (jak czytanie menu):
./SM2024-Projekt.out info -i image.nf
```

## 🐠 Dostępne opcje (jak menu North Fish)

### Typy obrazów (-t):

- `rgb555` - Jak panierowany dorsz - lekki i chrupiący
- `rgb888` - Jak łosoś premium - pełna jakość
- `ycbcr` - Jak ryba z frytkami - zoptymalizowana kompozycja

### Filtry (-f):

- `none` - Bez filtra, jak świeża ryba prosto z morza
- `average` - Wygładzony jak kremowy sos rybny

### Kompresja (-c):

- `none` - Bez kompresji, jak XXL porcja w North Fish
- `dct` - Kompresja DCT, jak zgrabnie zapakowane fish & chips
- `dct_chroma` - DCT z subsamplingiem chrominancji (tylko dla YCbCr, jak promocja 2+1)
- `rle` - Kompresja RLE, jak szybka dostawa z North Fish

## 🎣 Testowanie wszystkich kombinacji

Dołączony skrypt `test_all_combinations.sh` pozwala przetestować wszystkie możliwe kombinacje ustawień - jak degustacja całego menu North Fish! Generuje on stronę HTML z podglądem wszystkich wariantów.

```bash
./test_all_combinations.sh
```

## 🦈 Dlaczego North Fish Format?

- **Wydajność:** Jak Rust jest szybki w działaniu, tak NF jest szybki w kompresji
- **Bezpieczeństwo:** Zero niespodzianek - jak świeża ryba z certyfikatem jakości
- **Elastyczność:** Różne tryby kompresji jak różne sosy do wyboru
- **Nowoczesność:** Współczesne podejście do kompresji obrazów, jak nowoczesne wnętrza North Fish

_Nie koduj na pusty żołądek! Podczas pracy z North Fish Format, zalecamy konsumpcję produktów North Fish dla optymalnej wydajności kodowania!_ 🐟
