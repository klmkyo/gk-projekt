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

  // TEST: Create a sample image (all black), save and read to/from file
  NFHeaderUser header;

  header.type = ImageType::RGB555;
  header.filter = FilterType::None;
  header.compression = CompressionType::None;
  header.width = 3;
  header.height = 3;
  header.subsamplingEnabled = false;

  Canvas image(header.height, std::vector<Color>(header.width, {0, 0, 0}));

  for (int i = 0; i < header.height; i++) {
    for (int j = 0; j < header.width; j++) {
      image[i][j] = {(Uint8)(rand() % 256), (Uint8)(rand() % 256)};

      // set to a color exactly representable in RGB555
      image[i][j] = {0x00, 0x00, 0x00};
    }
  }

  saveNFImage("test.nf", header, image);

  auto [loadedHeader, loadedImage] = loadNFImage("test.nf");

  // DEBUG print data about the loaded image
  std::cout << "Loaded image header: " << std::endl;
  std::cout << "Version: " << (int)loadedHeader.version << std::endl;
  std::cout << "Type: " << (int)loadedHeader.type << std::endl;
  std::cout << "Filter: " << (int)loadedHeader.filter << std::endl;
  std::cout << "Compression: " << (int)loadedHeader.compression << std::endl;
  std::cout << "Width: " << loadedHeader.width << std::endl;
  std::cout << "Height: " << loadedHeader.height << std::endl;
  std::cout << "Subsampling enabled: " << loadedHeader.subsamplingEnabled
            << std::endl;

  std::cout << "Loaded image data: " << std::endl;
  std::cout << "Width: " << loadedImage[0].size() << std::endl;
  std::cout << "Height: " << loadedImage.size() << std::endl;

  // compare canvases.
  bool isDifferent = false;

  for (int i = 0; i < header.height; i++) {
    for (int j = 0; j < header.width; j++) {
      if (image[i][j] != loadedImage[i][j])
        isDifferent = true;

      // same as above, but print decimal and hex
      std::cout << "Images are different at pixel (" << j << ", " << i
                << "). Expected: " << (int)image[i][j].r << ", "
                << (int)image[i][j].g << ", " << (int)image[i][j].b
                << " \tGot: " << (int)loadedImage[i][j].r << ", "
                << (int)loadedImage[i][j].g << ", " << (int)loadedImage[i][j].b
                << std::hex << " \t(" << (int)image[i][j].r << ", "
                << (int)image[i][j].g << ", " << (int)image[i][j].b << ") \t("
                << (int)loadedImage[i][j].r << "," << (int)loadedImage[i][j].g
                << "," << (int)loadedImage[i][j].b << ")" << std::dec
                << std::endl;
    }
  }

  if (isDifferent) {
    std::cout << "Images are different!" << std::endl;
  } else {
    std::cout << "Images are the same!" << std::endl;
  }
  return 0;

  return 0;
}