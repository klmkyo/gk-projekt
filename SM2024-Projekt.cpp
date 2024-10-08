#include "SM2024-Zmienne.h"
#include "SM2024-Funkcje.h"
#include "SM2024-MedianCut.h"
#include "SM2024-Pliki.h"
#include "SM2024-Gui.h"
#include <iostream>
#include <map>
#include <string>

typedef std::map<int, std::vector<std::string>> CommandAliasMap;

template <typename T>
using ParameterMap = std::map<char, T>;

int findCommand(CommandAliasMap &commandsAliases, std::string command) {
    for (auto &aliases : commandsAliases) {
        for (auto &alias : aliases.second) {
            if (alias == command) return aliases.first;
        }
    }
    return 0;
}

void readParameterMap(ParameterMap<std::string> &parameterMap, int offset,
                      int argc, char *argv[]) {
    for (int i = offset; i < argc; i++) {
        if (strlen(argv[i]) < 2) continue;
        if (argv[i][0] == '-' && i + 1 < argc)
            parameterMap[argv[i][1]] = std::string(argv[i + 1]);
    }
}

bool hasParameter(ParameterMap<std::string> &parameterMap, char parameter) {
    return parameterMap.find(parameter) != parameterMap.end();
}

int main(int argc, char *argv[]) {
    CommandAliasMap commandsAliases;

    /* tobmp - odczytuje plik kfc, zapisuje plik bmp */
    commandsAliases[1] = {"tobmp", "-t", "-tobmp"};
    /* frombmp - odczytuje plik bmp, zapisuje plik kfc */
    commandsAliases[2] = {"frombmp", "-f", "-frombmp"};

    const std::string appName = argc < 1 ? "kfc" : argv[0];

    // If no argv start gui
    if (argc == 1) {
        startupGui();
        return 0;
    }

    /* Wypisuje wszystkie dostępne komendy bez opisu */
    if (argc <= 1 || (argc == 2 && (std::string(argv[1]) == "help" ||
                                    std::string(argv[1]) == "-help"))) {
        std::cout << "  Witamy w konwerterze obrazów 🍗 KFC <-> 🎨 BMP.\n"
                  << "Dostępne operacje:\n"
                  << "1. Konwersja formatu KFC na BMP\n"
                  << "> " << appName
                  << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
                  << "Wyświetl więcej informacji używając '" << appName
                  << " -help tobmp'\n"
                  << "2. Konwersja formatu BMP na KFC\n"
                  << "> " << appName
                  << " frombmp <ścieżka_pliku_bmp> [-s ścieżka_pliku_kfc] [-t "
                     "tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
                  << "Wyświetl więcej informacji używając '" << appName
                  << " -help tobmp'\n";
    }

    /* W przypadku wysłania 'kfc help <command_name>' wyświetlony zostanie opis
       komendy */
    else if (argc == 3 && (std::string(argv[1]) == "help" ||
                           std::string(argv[1]) == "-help")) {
        int primaryCommandId = findCommand(commandsAliases, argv[2]);
        switch (primaryCommandId) {
            case 1: { /* tobmp */
                std::cout
                    << "> " << appName
                    << " tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp]\n"
                    << "Opis: Komenda 'tobmp' konwertuje plik w formacie KFC "
                       "na format BMP \n"
                    << "Parametry obowiązkowe:\n"
                    << "\t<ścieżka_pliku_kfc> - ścieżka do pliku w formacie "
                       "kfc (relatywna lub absolutna)\n"
                    << "Parametry opcjonalne:\n"
                    << "\t[-s ścieżka_pliku_bmp] - ścieżka do nowo utworzonego "
                       "pliku (domyślnie plik kfc ze zmienionym "
                       "rozszerzeniem)\n";
                break;
            }
            case 2: { /* frombmp*/
                std::cout
                    << "> " << appName
                    << " frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] "
                       "[-t tryb(1-5)] [-d dithering(none/bayer/floyd)]\n"
                    << "Opis: Komenda 'frombmp' konwertuje plik w formacie BMP "
                       "na format KFC \n"
                    << "Parametry obowiązkowe:\n"
                    << "\t<ścieżka_pliku_bmp> - ścieżka do pliku w formacie "
                       "bmp (relatywna lub absolutna)\n"
                    << "Parametry opcjonalne:\n"
                    << "\t[-s ścieżka_pliku_kfc] - ścieżka do nowo utworzonego "
                       "pliku (domyślnie plik bmp ze zmienionym "
                       "rozszerzeniem)\n"
                    << "\t[-t tryb(1-5)] - tryb konwersji obrazu (domyślnie "
                       "1), dostępne tryby:\n"
                    << "\t\t1 - Paleta narzucona\n"
                    << "\t\t2 - Szarość narzucona\n"
                    << "\t\t3 - Paleta wykryta\n"
                    << "\t\t4 - Szarość wykryta\n"
                    << "\t\t5 - Paleta dedykowana\n"
                    << "\t[-d dithering(none/bayer/floyd)] - tryb ditheringu "
                       "(domyślnie none - bez ditheringu)\n";
                break;
            }
            default: {
                std::cout
                    << "Nieznana komenda. Użyj '" << appName
                    << " help' aby dowiedzieć się o istniejących komendach."
                    << std::endl;
                break;
            }
        }
    }

    /* W pozostałych przypadkach będzie próba rozpoznania komendy z 1 argumentu
       i jej wykonanie */
    else if (argc > 1) {
        int primaryCommandId = findCommand(commandsAliases, argv[1]);

        switch (primaryCommandId) {
            case 1: { /* tobmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] */
                if (argc < 3) {
                    std::cout << "Nie podano ścieżki do pliku kfc. Użyj '"
                              << appName
                              << " help tobmp' aby dowiedzieć się więcej."
                              << std::endl;
                    break;
                }
                std::string kfcPath = argv[2];
                ParameterMap<std::string> parameterMap;
                readParameterMap(parameterMap, 3, argc, argv);
                /* parametr s - scieżka pliku kfc */
                std::string bmpPath =
                    hasParameter(parameterMap, 's')
                        ? parameterMap['s']
                        : kfcPath.substr(0, kfcPath.find_last_of('.')) + ".bmp";

                KonwertujKfcNaBmp(kfcPath, bmpPath);
                break;
            }
            case 2: { /* frombmp <ścieżka_pliku_kfc> [-s ścieżka_pliku_bmp] [-t
                         tryb(1-5)] [-d dithering(none/bayer/floyd)] */
                if (argc < 3) {
                    std::cout << "Nie podano ścieżki do pliku bmp. Użyj '"
                              << appName
                              << " help frombmp' aby dowiedzieć się więcej."
                              << std::endl;
                    break;
                }
                std::string bmpPath = argv[2];
                ParameterMap<std::string> parameterMap;
                readParameterMap(parameterMap, 3, argc, argv);
                /* parametr s - scieżka pliku kfc */
                std::string kfcPath =
                    hasParameter(parameterMap, 's')
                        ? parameterMap['s']
                        : bmpPath.substr(0, bmpPath.find_last_of('.')) + ".kfc";
                /* parametr t - tryb obrazu */
                TrybObrazu tryb = TrybObrazu::PaletaDedykowana;
                if (hasParameter(parameterMap, 't')) {
                    int _tryb = std::stoi(parameterMap['t']);
                    if (_tryb < 1 || _tryb > 5) {
                        std::cout << "Nieprawidłowy tryb konwersji. Użyj '"
                                  << appName
                                  << " help frombmp' aby dowiedzieć się więcej."
                                  << std::endl;
                        break;
                    }
                    tryb = static_cast<TrybObrazu>(_tryb);
                }
                /* parametr d - dithering*/
                Dithering dithering = Dithering::Brak;
                if (hasParameter(parameterMap, 'd')) {
                    std::string _dithering = parameterMap['d'];
                    if (_dithering == "none")
                        dithering = Dithering::Brak;
                    else if (_dithering == "bayer")
                        dithering = Dithering::Bayer;
                    else if (_dithering == "floyd")
                        dithering = Dithering::Floyd;
                    else {
                        std::cout << "Nieprawidłowy tryb ditheringu. Użyj '"
                                  << appName
                                  << " help frombmp' aby dowiedzieć się więcej."
                                  << std::endl;
                        break;
                    }
                }
                KonwertujBmpNaKfc(bmpPath, kfcPath, tryb, dithering);
                break;
            }
            default: {
                std::cout << "Nieznana komenda. Użyj '" << appName
                          << " help' aby dowiedzieć się o dostępnych komendach."
                          << std::endl;
                break;
            }
        }
    }
    return 0;
}