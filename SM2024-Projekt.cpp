#include "SM2024-Gui.h"
#include "SM2024-Pliki.h"
#include "SM2024-Zmienne.h"
#include <iostream>
#include <map>
#include <string>

typedef std::map<int, std::vector<std::string>> CommandAliasMap;

template <typename T> using ParameterMap = std::map<char, T>;

int findCommand(CommandAliasMap &commandsAliases, const std::string &command) {
  for (auto &aliases : commandsAliases) {
    for (auto &alias : aliases.second) {
      if (alias == command) {
        return aliases.first;
      }
    }
  }
  return 0;
}

void readParameterMap(ParameterMap<std::string> &parameterMap, int offset,
                      int argc, char *argv[]) {
  for (int i = offset; i < argc; i++) {
    if (strlen(argv[i]) < 2) {
      continue;
    }
    if (argv[i][0] == '-' && i + 1 < argc) {
      parameterMap[argv[i][1]] = std::string(argv[i + 1]);
    }
  }
}

bool hasParameter(ParameterMap<std::string> &parameterMap, char parameter) {
  return parameterMap.find(parameter) != parameterMap.end();
}

int main(int argc, char *argv[]) {

  // // TEST: Create a sample image (all black), save and read to/from file
  // using saveNFImage and loadNFImage

  // NFHeaderUser header;

  // header.type = ImageType::RGB555;
  // header.filter = FilterType::None;
  // header.compression = CompressionType::None;
  // header.width = 3;
  // header.height = 3;
  // header.subsamplingEnabled = false;

  // Canvas image(header.height, std::vector<Color>(header.width, {0, 0, 0}));

  // for (int i = 0; i < header.height; i++) {
  //     for (int j = 0; j < header.width; j++) {
  //         // image[i][j] = {(Uint8)(rand() % 256), (Uint8)(rand() % 256),
  //         (Uint8)(rand() % 256)};

  //         // set to hex AA
  //         image[i][j] = {0xAA, 0xAA, 0xAA};
  //     }
  // }

  // saveNFImage("test.nf", header, image);

  // auto [loadedHeader, loadedImage] = loadNFImage("test.nf");

  // // DEBUG print data about the loaded image
  // std::cout << "Loaded image header: " << std::endl;
  // std::cout << "Version: " << (int)loadedHeader.version << std::endl;
  // std::cout << "Type: " << (int)loadedHeader.type << std::endl;
  // std::cout << "Filter: " << (int)loadedHeader.filter << std::endl;
  // std::cout << "Compression: " << (int)loadedHeader.compression << std::endl;
  // std::cout << "Width: " << loadedHeader.width << std::endl;
  // std::cout << "Height: " << loadedHeader.height << std::endl;
  // std::cout << "Subsampling enabled: " << loadedHeader.subsamplingEnabled <<
  // std::endl;

  // std::cout << "Loaded image data: " << std::endl;
  // std::cout << "Width: " << loadedImage[0].size() << std::endl;
  // std::cout << "Height: " << loadedImage.size() << std::endl;

  // // compare canvases.
  // bool isDifferent = false;

  // for (int i = 0; i < header.height; i++) {
  //     for (int j = 0; j < header.width; j++) {
  //         if (image[i][j] != loadedImage[i][j])
  //             isDifferent = true;

  //             // same as above, but print decimal and hex
  //             std::cout << "Images are different at pixel (" << j << ", " <<
  //             i << "). Expected: " << (int)image[i][j].r << ", " <<
  //             (int)image[i][j].g << ", " << (int)image[i][j].b << " \tGot: "
  //             << (int)loadedImage[i][j].r << ", " << (int)loadedImage[i][j].g
  //             << ", " << (int)loadedImage[i][j].b << std::hex << " \t(" <<
  //             (int)image[i][j].r << ", " << (int)image[i][j].g << ", " <<
  //             (int)image[i][j].b << ") \t(" << (int)loadedImage[i][j].r << ",
  //             " << (int)loadedImage[i][j].g << ", " <<
  //             (int)loadedImage[i][j].b << ")" << std::dec << std::endl;
  //         }
  //     }
  // }

  // if (isDifferent) {
  //     std::cout << "Images are different!" << std::endl;
  // } else {
  //     std::cout << "Images are the same!" << std::endl;
  // }
  // return 0;

  CommandAliasMap commandsAliases;

  /* tobmp - reads NF file, saves BMP file */
  commandsAliases[1] = {"tobmp", "-t", "-tobmp"};
  /* frombmp - reads BMP file, saves NF file */
  commandsAliases[2] = {"frombmp", "-f", "-frombmp"};

  const std::string appName = argc < 1 ? "nf" : argv[0];

  // If no argv start gui
  if (argc == 1) {
    startupGui();
    return 0;
  }

  /* Print all available commands without description */
  if (argc <= 1 || (argc == 2 && (std::string(argv[1]) == "help" ||
                                  std::string(argv[1]) == "-help"))) {
    std::cout << "  Welcome to the 🎨 BMP <-> 🖼️ NF image converter.\n"
              << "Available operations:\n"
              << "1. Convert NF format to BMP\n"
              << "> " << appName << " tobmp <nf_file_path> [-s bmp_file_path]\n"
              << "Show more information using '" << appName << " -help tobmp'\n"
              << "2. Convert BMP format to NF\n"
              << "> " << appName
              << " frombmp <bmp_file_path> [-s nf_file_path] [-t type] [-f "
                 "filter] [-c compression] [-ss subsampling]\n"
              << "Show more information using '" << appName
              << " -help frombmp'\n";
  }

  /* If 'nf help <command_name>' is sent, command description will be displayed
   */
  else if (argc == 3 && (std::string(argv[1]) == "help" ||
                         std::string(argv[1]) == "-help")) {
    int primaryCommandId = findCommand(commandsAliases, argv[2]);
    switch (primaryCommandId) {
    case 1: { /* tobmp */
      std::cout << "> " << appName
                << " tobmp <nf_file_path> [-s bmp_file_path]\n"
                << "Description: 'tobmp' command converts NF format file to "
                   "BMP format\n"
                << "Required parameters:\n"
                << "\t<nf_file_path> - path to NF format file (relative or "
                   "absolute)\n"
                << "Optional parameters:\n"
                << "\t[-s bmp_file_path] - path to newly created file "
                   "(default: NF file with changed extension)\n";
      break;
    }
    case 2: { /* frombmp */
      std::cout
          << "> " << appName
          << " frombmp <bmp_file_path> [-s nf_file_path] [-t type] [-f filter] "
             "[-c compression] [-ss subsampling]\n"
          << "Description: 'frombmp' command converts BMP format file to NF "
             "format\n"
          << "Required parameters:\n"
          << "\t<bmp_file_path> - path to BMP format file (relative or "
             "absolute)\n"
          << "Optional parameters:\n"
          << "\t[-s nf_file_path] - path to newly created file (default: BMP "
             "file with changed extension)\n"
          << "\t[-t type] - image type (default: RGB555), available types:\n"
          << "\t\tYUV - YUV color space\n"
          << "\t\tYIQ - YIQ color space\n"
          << "\t\tYCbCr - YCbCr color space\n"
          << "\t\tHSL - HSL color space\n"
          << "\t\tRGB555 - 15-bit RGB\n"
          << "\t\tRGB565 - 16-bit RGB\n"
          << "\t[-f filter] - filter type (default: none)\n"
          << "\t[-c compression] - compression type (default: none)\n"
          << "\t[-ss] - enable subsampling (default: disabled)\n";
      break;
    }
    default: {
      std::cout << "Unknown command. Use '" << appName
                << " help' to learn about existing commands." << std::endl;
      break;
    }
    }
  }

  /* In other cases, try to recognize command from 1st argument and execute it
   */
  else if (argc > 1) {
    int primaryCommandId = findCommand(commandsAliases, argv[1]);

    switch (primaryCommandId) {
    case 1: { /* tobmp <nf_file_path> [-s bmp_file_path] */
      if (argc < 3) {
        std::cout << "No NF file path provided. Use '" << appName
                  << " help tobmp' to learn more." << '\n';
        break;
      }
      std::string nfPath = argv[2];
      ParameterMap<std::string> parameterMap;
      readParameterMap(parameterMap, 3, argc, argv);

      std::string bmpPath =
          hasParameter(parameterMap, 's')
              ? parameterMap['s']
              : nfPath.substr(0, nfPath.find_last_of('.')) + ".bmp";

      auto [header, image] = loadNFImage(nfPath);
      ZapiszCanvasDoBmp(image, bmpPath);
      break;
    }
    case 2: { /* frombmp <bmp_file_path> [-s nf_file_path] [-t type] [-f filter]
                 [-c compression] [-ss] */
      if (argc < 3) {
        std::cout << "No BMP file path provided. Use '" << appName
                  << " help frombmp' to learn more." << '\n';
        break;
      }
      std::string bmpPath = argv[2];
      ParameterMap<std::string> parameterMap;
      readParameterMap(parameterMap, 3, argc, argv);

      std::string nfPath =
          hasParameter(parameterMap, 's')
              ? parameterMap['s']
              : bmpPath.substr(0, bmpPath.find_last_of('.')) + ".nf";

      NFHeaderUser header;
      header.type = ImageType::RGB555; // default
      header.filter = FilterType::None;
      header.compression = CompressionType::None;
      header.subsamplingEnabled = false;

      if (hasParameter(parameterMap, 't')) {
        std::string type = parameterMap['t'];
        if (type == "YUV")
          header.type = ImageType::YUV;
        else if (type == "YIQ")
          header.type = ImageType::YIQ;
        else if (type == "YCbCr")
          header.type = ImageType::YCbCr;
        else if (type == "HSL")
          header.type = ImageType::HSL;
        else if (type == "RGB555")
          header.type = ImageType::RGB555;
        else if (type == "RGB565")
          header.type = ImageType::RGB565;
        else {
          std::cout << "Invalid image type. Use '" << appName
                    << " help frombmp' to learn more." << '\n';
          break;
        }
      }

      if (hasParameter(parameterMap, 'f')) {
        std::string filter = parameterMap['f'];
        if (filter == "none")
          header.filter = FilterType::None;
        else if (filter == "sub")
          header.filter = FilterType::Sub;
        else if (filter == "up")
          header.filter = FilterType::Up;
        else if (filter == "average")
          header.filter = FilterType::Average;
        else if (filter == "paeth")
          header.filter = FilterType::Paeth;
        else {
          std::cout << "Invalid filter type. Use '" << appName
                    << " help frombmp' to learn more." << '\n';
          break;
        }
      }

      header.subsamplingEnabled = hasParameter(parameterMap, 'ss');

      Canvas image = ladujBMPDoPamieci(bmpPath);
      header.width = image[0].size();
      header.height = image.size();

      saveNFImage(nfPath, header, image);
      break;
    }
    default: {
      std::cout << "Unknown command. Use '" << appName
                << " help' to learn about available commands." << std::endl;
      break;
    }
    }
  }
  return 0;
}