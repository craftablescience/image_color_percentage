#include <argparse/argparse.hpp>
#include <sourcepp/FS.h>
#include <sourcepp/String.h>
#include <vtfpp/ImageConversion.h>

using namespace sourcepp;
using namespace vtfpp;

#ifdef _WIN32
#define CP_UTF8 65001
extern "C" __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int);
#endif

namespace {

bool closeEnough(ImagePixel::RGB888 pixel, int r, int g, int b, int variance) {
	return
		(pixel.r >= r - variance && r + variance >= pixel.r) &&
		(pixel.g >= g - variance && g + variance >= pixel.g) &&
		(pixel.b >= b - variance && b + variance >= pixel.b);
}

} // namespace

int main(int argc, const char* const argv[]) {
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8); // Set up console to show UTF-8 characters
	setvbuf(stdout, nullptr, _IOFBF, 1000); // Enable buffering so VS terminal won't chop up UTF-8 byte sequences
#endif

	argparse::ArgumentParser cli{"image_color_percentage", "v0.1.0", argparse::default_arguments::help};

#ifdef _WIN32
	// Add the Windows-specific ones because why not
	cli.set_prefix_chars("-/");
	cli.set_assign_chars("=:");
#endif

	std::string inputPath;
	cli
		.add_argument("path")
		.metavar("PATH")
		.help("The path to the input image file.")
		.required()
		.store_into(inputPath);

	int rowCount;
	cli
		.add_argument("-c", "--row-count")
		.metavar("COUNT")
		.help("The number of rows in the image file to process. If it does not cleanly divide the input image height, the image will be resized upwards until it does.")
		.default_value(1)
		.scan<'d', int>()
		.store_into(rowCount);

	int red = 0;
	cli
		.add_argument("-r", "--red")
		.metavar("RED")
		.help("The amount of red target pixels have. Ranges from 0-255.")
		.required()
		.scan<'d', int>()
		.store_into(red);

	int green = 0;
	cli
		.add_argument("-g", "--green")
		.metavar("GREEN")
		.help("The amount of green target pixels have. Ranges from 0-255.")
		.required()
		.scan<'d', int>()
		.store_into(green);

	int blue = 0;
	cli
		.add_argument("-b", "--blue")
		.metavar("BLUE")
		.help("The amount of blue target pixels have. Ranges from 0-255.")
		.required()
		.scan<'d', int>()
		.store_into(blue);

	int variance = 8;
	cli
		.add_argument("-v", "--variance")
		.metavar("VARIANCE")
		.help("The amount that red, green, and blue in a pixel can deviate from the search color.")
		.default_value(8)
		.scan<'d', int>()
		.store_into(variance);

	bool save = false;
	cli
		.add_argument("-s", "--save")
		.help("Save each row as individual images.")
		.flag()
		.store_into(save);

	bool debug = false;
	cli
		.add_argument("-d", "--debug")
		.help("When a pixel counts toward the search color, make it bright pink and save a copy of the base image.")
		.flag()
		.store_into(debug);

	try {
		cli.parse_args(argc, argv);
	} catch (...) {
		std::cerr << cli << std::endl;
		return EXIT_FAILURE;
	}

	ImageFormat originalFormat;
	int width, oldHeight, frameCount;
	auto originalData = ImageConversion::convertFileToImageData(fs::readFileBuffer(inputPath), originalFormat, width, oldHeight, frameCount);
	int height = oldHeight;
	if (height % rowCount != 0) {
		height += height % rowCount;
		originalData = ImageConversion::resizeImageData(originalData, originalFormat, width, width, oldHeight, height, true, ImageConversion::ResizeFilter::MITCHELL);
	}

	auto data = ImageConversion::convertImageDataToFormat({originalData.data(), originalData.size() / frameCount}, originalFormat, ImageFormat::RGB888, width, height);
	for (int i = 0; i < rowCount; i++) {
		const auto offsetBegin = width * i * (height / rowCount) * (ImageFormatDetails::bpp(ImageFormat::RGB888) / 8);
		const auto offsetEnd = width * (i + 1) * (height / rowCount) * (ImageFormatDetails::bpp(ImageFormat::RGB888) / 8);
		std::span<std::byte> currentData{data.data() + offsetBegin, data.data() + offsetEnd};

		// Calculations
		int counter = 0;
		std::span<ImagePixel::RGB888> currentDataRGB888{reinterpret_cast<ImagePixel::RGB888*>(currentData.data()), currentData.size() / sizeof(ImagePixel::RGB888)};
		for (auto& pixel : currentDataRGB888) {
			if (save) {
				fs::writeFileBuffer(std::filesystem::path{inputPath}.replace_extension().string() + string::padNumber(i + 1, 2) + ".png", ImageConversion::convertImageDataToFile(currentData, ImageFormat::RGB888, width, height / rowCount, ImageConversion::FileFormat::PNG));
			}
			if (closeEnough(pixel, red, green, blue, variance)) {
				counter++;
				pixel.r = 0xff;
				pixel.g = 0x00;
				pixel.b = 0x88;
			}
		}
		std::cout << "Row " << string::padNumber(i + 1, 2) << ": " << static_cast<float>(counter) / static_cast<float>(currentDataRGB888.size()) * 100 << "%" << std::endl;
	}
	if (debug) {
		fs::writeFileBuffer(std::filesystem::path{inputPath}.replace_extension().string() + "_debug.png", ImageConversion::convertImageDataToFile(data, ImageFormat::RGB888, width, height, ImageConversion::FileFormat::PNG));
	}

	return EXIT_SUCCESS;
}
