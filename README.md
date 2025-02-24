# image_color_percentage

Find how often a given color occurs in a given image.
Allows for breaking the image into multiple rows.

## Credits

- Dependencies:
  - [argparse](https://github.com/p-ranav/argparse)
  - [sourcepp](https://github.com/craftablescience/sourcepp)

## Help Text

```
Usage: image_color_percentage [--help] [--row-count COUNT] --red RED --green GREEN --blue BLUE
                              [--variance VARIANCE] [--save] [--debug] PATH

Positional arguments:
  PATH               The path to the input image file. [required]

Optional arguments:
  -h, --help         shows help message and exits
  -c, --row-count    The number of rows in the image file to process. If it does not cleanly divide the
                     input image height, the image will be resized upwards until it does. [default: 1]
  -r, --red RED      The amount of red target pixels have. Ranges from 0-255. [required]
  -g, --green GREEN  The amount of green target pixels have. Ranges from 0-255. [required]
  -b, --blue BLUE    The amount of blue target pixels have. Ranges from 0-255. [required]
  -v, --variance     The amount that red, green, and blue in a pixel can deviate from the search color.
                     [default: 8]
  -s, --save         Save each row as individual images.
  -d, --debug        When a pixel counts toward the search color, make it bright pink and save a copy
                     of the base image.
```
