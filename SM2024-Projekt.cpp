#include "SM2024-Pliki.h"
#include "SM2024-Zmienne.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>

typedef std::map<int, std::vector<std::string>> CommandAliasMap;

template <typename T> using ParameterMap = std::map<char, T>;

void printUsage() {
  std::cout
      << "Usage: SM2024-Projekt <command> [options]\n\n"
      << "Commands:\n"
      << "  convert, c     Convert between BMP and NF formats\n"
      << "  view, v        View an NF image\n"
      << "  info, i        Display information about an NF image\n"
      << "\nOptions for 'convert':\n"
      << "  -i <path>      Input file path (BMP or NF format)\n"
      << "  -o <path>      Output file path (NF or BMP format)\n"
      << "  -t <type>      Image type for NF output (rgb555, rgb888, ycbcr)\n"
      << "  -f <filter>    Filter type for NF output (none, average)\n"
      << "  -c <comp>      Compression for NF output (none, dct, dct_chroma)\n"
      << "\nOptions for 'view':\n"
      << "  -i <path>      Input NF image path\n"
      << "\nOptions for 'info':\n"
      << "  -i <path>      Input NF image path\n"
      << "\nExamples:\n"
      << "  SM2024-Projekt convert -i input.bmp -o output.nf -t rgb555 -f none "
         "-c none\n"
      << "  SM2024-Projekt convert -i input.nf -o output.bmp\n"
      << "  SM2024-Projekt view -i image.nf\n"
      << "  SM2024-Projekt info -i image.nf\n";
}

ImageType parseImageType(const std::string &type) {
  if (type == "rgb555")
    return ImageType::RGB555_WITH_BAYER_DITHERING;
  if (type == "rgb888")
    return ImageType::RGB888;
  if (type == "ycbcr")
    return ImageType::YCbCr888;
  throw std::invalid_argument("Invalid image type: " + type);
}

FilterType parseFilterType(const std::string &filter) {
  if (filter == "none")
    return FilterType::None;
  if (filter == "average")
    return FilterType::Average;
  throw std::invalid_argument("Invalid filter type: " + filter);
}

CompressionType parseCompressionType(const std::string &comp) {
  if (comp == "none")
    return CompressionType::None;
  if (comp == "dct")
    return CompressionType::Dct;
  if (comp == "dct_chroma")
    return CompressionType::DctPlusChromaSubsampling;
  if (comp == "rle")
    return CompressionType::RLE;
  throw std::invalid_argument("Invalid compression type: " + comp);
}

Canvas loadBMPToCanvas(const std::string &filename) {
  SDL_Surface *surface = SDL_LoadBMP(filename.c_str());
  if (!surface) {
    throw std::runtime_error("Failed to load BMP: " +
                             std::string(SDL_GetError()));
  }

  Canvas canvas(surface->h, std::vector<Color>(surface->w));
  for (int y = 0; y < surface->h; y++) {
    for (int x = 0; x < surface->w; x++) {
      SDL_Color pixel = getPixelSurface(x, y, surface);
      canvas[y][x] = {pixel.r, pixel.g, pixel.b};
    }
  }

  SDL_FreeSurface(surface);
  return canvas;
}

void saveCanvasToBMP(const Canvas &canvas, const std::string &filename) {
  int width = canvas[0].size();
  int height = canvas.size();

  SDL_Surface *surface = SDL_CreateRGBSurface(
      0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

  if (!surface) {
    throw std::runtime_error("Failed to create surface: " +
                             std::string(SDL_GetError()));
  }

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Uint32 pixel = SDL_MapRGB(surface->format, canvas[y][x].r, canvas[y][x].g,
                                canvas[y][x].b);

      Uint32 *target_pixel =
          (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch + x * 4);
      *target_pixel = pixel;
    }
  }

  SDL_SaveBMP(surface, filename.c_str());
  SDL_FreeSurface(surface);
}

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

bool hasExtension(const std::string &filename, const std::string &ext) {
  if (filename.length() < ext.length())
    return false;
  return std::equal(
      ext.rbegin(), ext.rend(), filename.rbegin(),
      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
    return 1;
  }

  CommandAliasMap commands = {
      {1, {"convert", "c"}}, {2, {"view", "v"}}, {3, {"info", "i"}}};

  std::string command = argv[1];
  int commandId = findCommand(commands, command);

  ParameterMap<std::string> params;
  readParameterMap(params, 2, argc, argv);

  try {
    switch (commandId) {
    case 1: { // convert
      if (!hasParameter(params, 'i') || !hasParameter(params, 'o')) {
        std::cerr << "Error: Input and output paths are required for convert "
                     "command\n";
        printUsage();
        return 1;
      }

      std::string inputPath = params['i'];
      std::string outputPath = params['o'];
      bool inputIsBMP = hasExtension(inputPath, ".bmp");
      bool outputIsBMP = hasExtension(outputPath, ".bmp");

      if (inputIsBMP == outputIsBMP) {
        std::cerr
            << "Error: Convert command requires one BMP and one NF file\n";
        return 1;
      }

      if (inputIsBMP) {
        // BMP to NF conversion
        Canvas image = loadBMPToCanvas(inputPath);

        // Create header with user parameters
        NFHeaderUser header;
        header.width = image[0].size();
        header.height = image.size();

        // Set defaults if parameters are not provided
        header.type = hasParameter(params, 't') ? parseImageType(params['t'])
                                                : ImageType::RGB888;

        header.filter = hasParameter(params, 'f') ? parseFilterType(params['f'])
                                                  : FilterType::None;

        header.compression = hasParameter(params, 'c')
                                 ? parseCompressionType(params['c'])
                                 : CompressionType::None;

        // Save as NF format
        saveNFImage(outputPath, header, image);
        std::cout << "Successfully converted " << inputPath << " to "
                  << outputPath << std::endl;
      } else {
        // NF to BMP conversion
        auto [header, image] = loadNFImage(inputPath);
        saveCanvasToBMP(image, outputPath);
        std::cout << "Successfully converted " << inputPath << " to "
                  << outputPath << std::endl;
      }
      break;
    }

    case 2: { // view
      if (!hasParameter(params, 'i')) {
        std::cerr << "Error: Input path is required for view command\n";
        printUsage();
        return 1;
      }

      // Load the NF image
      auto [header, image] = loadNFImage(params['i']);

      // Save as temporary BMP for viewing
      std::string tempFile = "temp_view.bmp";
      saveCanvasToBMP(image, tempFile);

      // Open with system default viewer (you might want to implement a proper
      // viewer)
      std::string command = "open " + tempFile;
      system(command.c_str());
      break;
    }

    case 3: { // info
      if (!hasParameter(params, 'i')) {
        std::cerr << "Error: Input path is required for info command\n";
        printUsage();
        return 1;
      }

      // Load and display header information
      auto [header, _] = loadNFImage(params['i']);

      std::cout << "Image Information:\n"
                << "Format version: " << (int)header.version << "\n"
                << "Image type: ";

      switch (header.type) {
      case ImageType::RGB555_WITH_BAYER_DITHERING:
        std::cout << "RGB555 with Bayer dithering";
        break;
      case ImageType::RGB888:
        std::cout << "RGB888";
        break;
      case ImageType::YCbCr888:
        std::cout << "YCbCr888";
        break;
      }

      std::cout << "\nFilter: "
                << (header.filter == FilterType::None ? "None" : "Average")
                << "\nCompression: ";

      switch (header.compression) {
      case CompressionType::None:
        std::cout << "None";
        break;
      case CompressionType::Dct:
        std::cout << "DCT";
        break;
      case CompressionType::DctPlusChromaSubsampling:
        std::cout << "DCT with chroma subsampling";
        break;
      case CompressionType::RLE:
        std::cout << "Run-Length Encoding";
        break;
      }

      std::cout << "\nDimensions: " << header.width << "x" << header.height
                << " pixels\n";
      break;
    }

    default:
      std::cerr << "Unknown command: " << command << std::endl;
      printUsage();
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  SDL_Quit();
  return 0;
}