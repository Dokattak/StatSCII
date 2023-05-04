# WINDOWS BUILD ONLY

# StatSCII <img src="https://img.shields.io/badge/version-v1.0.0-brightgreen.svg">
<img src="https://img.shields.io/badge/language-C%2FC%2B%2B-blue.svg">

> StatSCII is a video to ASCII converter with aesthetic features such as color, grayscale mode, and average color, etc. StatSCII comes as either a stand-alone executable already compiled and linked, or as a single header file with the required OpenCV libraries(see <a href="#libraries">Libraries</a>)

## Installation (Pre-Compiled)

> This instruction section focuses on using the already compiled(pre-compiled) version of StatSCII. Please follow the numbered instructions below.

1. Download a released pre-compiled version of StatSCII.
2. Extract the contents to anywhere of your choice.
3. Ensure that all libraries are with the StatSCII executable(see <a href="#libraries">Libraries</a> for a list).
4. Open the command terminal(Right click Windows icon -> Terminal\Powershell)
5. Run StatSCII via the terminal.

## Installation (Header File)

> This instruction section focuses on adding `statscii.h` to a C++ project.

1. Download a released StatSCII header file.
2. Add the header file to a directory that can be accessed by the C++ project.
3. Include the header file into your project.

#### Run StatSCII via Command Prompt

6. Copy the path to the StatSCII executable(ex. C:\\path\\to\\statscii).
7. Type the following, replacing `[path]` with the copied path:

> `cd "[path]"`
> 
> NOTE: Ensure that the path is enclosed with only *one* pair of double quotations.

8. Run `.\StatSCII` or another method to execute StatSCII. If a help prompt is recieved then StatSCII is functioning correctly.*

#### \* Note

> If StatSCII cannot convert a file due to permission restrictions, close the command prompt, and re-open another as "Administor"
> 
> See <a href="#commands">Commands</a> to view avaliable commands, or read the help prompt by calling the StatSCII executable with *no* arguements.
> 
> **The resulting file is in the same place as the executable, and it is named "result.mp4"**

#### Using the Header File

> In order to use StatSCII, the StatSCII class must be initalized with a file path to a `.ttf` font file, and the path to the `mapfile`.
> Whichever map file is given, StatSCII will try to use it.
> 
> There are multiple public variables that can be changed. These dictate what happens during coversion(see <a href="#variables">Variables</a>).
> 
> Below is an example of StatSCII

```
// Create the StatSCII object
statscii stcii("font.ttf", "mapfile.txt");

// "Calibrates" StatSCII
stcii.calibrate(0, 0, 0.5, RelativeLuminance);

char videopath[] = "myvideo.mp4";

// Convert the video to ASCII
// The result is written in the executable's directory, and named as "result.mp4" 
stcii.convert(videopath, 100);
```

## Variables

> StatSCII uses variables in it's conversion process. These can be chanegd at any time, given that the types match. Below is a list of variables and what they dictate.
> 
> For `gstype`, please see <a href="#grayscale-types">Grayscale Types</a>

#### Class Variables

  - textcolor(`cv::Scalar`)
    > The color, including alpha, of the resulting text.


  - bgcolor(`cv::Scalar`)
    > The background, including alpha, of the resulting video.


  - gstype(`GSType`)
    > The type of grayscale conversion to use before subsituting a character


  - color(`bool`)
      > If true, the resulting character will inherit the average color of the space it was subituted in.


  - apply_static(`bool`)
    > Applies StatSCII's "static" variation of the ASCII art.


  - threshold(`int`)
    > Determines how many characters will be considered during subsitution.


#### Grayscale Types

  - `RelativeLuminance`
  - `Lightness`
  - `Contrast`
  - `Average`

## Calibration

> StatSCII cannot get the ranges of each character out of thin air. It needs to manually create an image of the character, which is a black character on a white background, and then gather the colors and average them. StatSCII then saves the result into the mapfile.
> 
> `overwrite_current_spectrum` determines if the current saved spectrum *in memory* is to be overwritten.
> 
> `overwrite_mapfile` determines if the current `mapfile.txt` is to be overwritten. Comments are saved when overwriting or not. If this variable is false, a new file called `new_mapfile.txt` will be created in the same area as the executable.
> 
> `bias` is a decimal that is multiplied to the resulting averaged color. This is due to StatSCII estimating the average too high, and so to level out the averages, a "bias" is applied to the average.
> 
> `grayscale_conversion_type` determines the method of RGB to grayscale.

## Commands

> The following is a list and discription of every command that the pre-compiled StatSCII recognizes.
> 
> The format for the commands is `[arguement] [value]`. Below is an example command that would be executed in the terminal.
```
.\StatSCII -src "C:\\path\\to\\myvideo.mp4" -chh 25 -col -cal -overmap -bis 0.4
```

> Below is the commands

  - `-src`
    > A file path; the path to the video that will be converted

  - `-fnt`
    > A file path; the path to a `.ttf` font file. This font will then be used in coversion.

  - `-spf`
    > A file path; the path to a `mapfile.txt`. You are not restricted to only the original `mapfile.txt`.
    
  - `-chh`
    > A number; the height, **in characters**, of the resulting video.

  - `-thr`
    > A number; the number of similar characters gathered before subsitution.

  - `-col`
    > None; if present, the resulting characters will inherit the average color that they took up.

  - `-gcv`
    > A number; The type of grayscale coversion to use. The types of grayscale conversion are as follows:
    > 
    > Relative Luminance = 0
    > Lightness = 1
    > Contrast = 2
    > Average = 3

  - `-stc`
    > None; if present, StatSCII will apply its 'ASCII Static' to the resulting video.
   
  - `-cal`
    > None; if present, Statscii will calibrate a new `mapfile.txt` and output it.

  - `-bis` *In relation to `-cal`*
    > A decimal; the bias applied to the average color value from calibration.

  - `-overmap` *In relation to `cal`*
    > None; if present, StatSCII will overwrite the *current* `mapfile.txt` in it's directory. If this is not specified, then StatSCII will output `new_mapfile.txt`.

#### Note

> StatSCII can both calibrate and covert at the same time. Calibration is preformed *first*, followed by conversion.

## Libraries

> StatSCII is designed to depend on the least amount of libraries possible. However, some limitations and extreme difficulties have lead it to depend solely on the 
> OpenCV library for reading, extracting, and writing from or to video files. All other content such as arrays and strings are done old-school. This eliminates the need 
> for other traditional C++ libraries like `string` and `vector`.
> 
> The primary reason for using old-school methods is the decrease in file sizes. Since StatSCII didn't need the majority of functions granted from libraries such as
> `string` and `vector`, it was decided that these libraries would be excluded at all costs.
> 
> Below are the OpenCV dependancies that must be present for StatSCII to function without error. Note that extension names are not included.

## OpenCV v4.7.0 Dependencies
  - `opencv_core470`
  - `opencv_freetype470`
  - `opencv_imgcodecs470`
  - `opencv_imgproc470`
  - `opencv_videoio470`

## Other Dependencies
  - `mapfile.txt`
  - `defaultfont.ttf`*

### \* Note
> You can specify the font file to use via the `-fnt` command. Othwerwise, StatSCII will try to default on `defaultfont.ttf`.
