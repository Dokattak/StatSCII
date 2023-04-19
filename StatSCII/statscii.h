#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <ctime>

#include <Windows.h>

#pragma warning(disable : 4996)
#pragma warning(disable: 6386)		// Too many false positives so I killed the warning
#pragma warning(disable: 6385)		// Too many false positives so I killed the warning

#define CALIBRATION_FONT_SIZE	 60
#define DEFAULT_CHARACTER		 "C"
#define BUFFER_FILE_SIZE		 500
#define TEMP_FILE_NAME			 "tmpfile.txt"
#define NEW_FILE_NAME			 "new_mapfile.txt"
#define STARTING_CHAR			 32
#define ENDING_CHAR				 126
#define RESULT_NAME				 "result.mp4"

namespace {
class _statexception : public std::exception
{
	const char* msg;
public:
	_statexception(const char* message) : msg(message) {}
	const char* what() { return msg; }
};

	const cv::Scalar _defaultcolor(255, 255, 255);
	const float _onethird = 1.0 / 3.0;

	int GetMax(int* _nums, int _size)
	{
		int result = _nums[0];
		for (int i = 1; i < _size; i++) { if (_nums[i] > result) result = _nums[i]; }

		return result;
	}
	int GetMin(int* _nums, int _size)
	{
		int result = _nums[0];
		for (int i = 1; i < _size; i++) { if (_nums[i] < result) result = _nums[i]; }

		return result;
	}
}

typedef int (*GrayScaleType)(int, int, int);
namespace statscii_grayscale_types
{
	int RelativeLuminance(int _r, int _g, int _b) { return 0.2126 * _r + 0.7152 * _g + 0.0722 * _b; }
	int Lightness(int _r, int _g, int _b)         { int n[3] = { _r, _g, _b }; return GetMax(n, 3) * 0.5 + GetMin(n, 3) * 0.5; }
	int Contrast(int _r, int _g, int _b)		  { return 0.299 * _r + 0.587 * _g + 0.114 * _b; }
	int Average(int _r, int _g, int _b)			  { return _r * _onethird + _g * _onethird + _b * _onethird; }
};

enum Variations {
	STANDARD,
	CLASSIC,
	CHAOS,
};

struct statscii
{
	statscii(char* _fontfile, char* _mapfile) : mapfile(_mapfile)
	{
		srand(time(0));

		// Sets font
		ft2->loadFontData(_fontfile, 0);

		FILE* file;								// Character map file
		char  text[BUFFER_FILE_SIZE];				// Line of file

		// If mapfile is null throw esception
		if (!(file = fopen(mapfile, "r"))) throw _statexception("Cannot open character map file");

		while (fgets(text, BUFFER_FILE_SIZE, file))
		{
			StripString(text);

			// Skip if first two chars are '/' or text is empty
			if (text[1] == '/' && text[0] == '/' || strlen(text) < 1) continue;

			if (!strlen(text)) continue;

			int num;
			CleanSpectrum(text, &num);
			spectrum[text[0] - STARTING_CHAR] = num;
		}

		fclose(file);
	}

	/// <summary>
	/// Converts a video file into a StatSCII modified *.mp4
	/// </summary>
	/// <param name="videofile">File path to source video</param>
	/// <param name="height">Height, in characters, of the resulting video</param>
	/// <param name="variation">Method of video conversion. See Variations enum for more conversions</param>
	/// <param name="grayscale_conversion_type">Method of grayscale conversion. See GrayScale types for more conversions</param>
	/// <param name="threshold">How many characters will be considered for video conversion. Only change if the variation is Variations::CHAOS</param>
	void convert(
		char* videofile,
		int height								= 50,
		bool color								= 0,
		Variations variation				    = Variations::STANDARD,
		GrayScaleType grayscale_conversion_type = GrayScaleType(statscii_grayscale_types::RelativeLuminance),
		int threshold							= 1
	) {
		printf("Creating video cpature...\n");
		cv::VideoCapture video(videofile);							// Original video
		if (!video.isOpened()) throw _statexception("Video cannot be opened.");

		double video_height = video.get(cv::CAP_PROP_FRAME_HEIGHT);	// Original video height in pixels
		double video_width	= video.get(cv::CAP_PROP_FRAME_WIDTH);	// Original video width in pixels
		cv::Size size(video_width, video_height);
		
		printf("Finding codec and creating video writer...\n");
		cv::VideoWriter result(										// Resulting video
			RESULT_NAME,
			-1,
			video.get(cv::CAP_PROP_FPS),
			size
		);
		
		if (!result.isOpened()) throw _statexception("Cannot create output video.");

		// Calculates dimensions for conversion
		_yitr = video_height / height;
		_xitr = ft2->getTextSize(DEFAULT_CHARACTER, _yitr, -1, 0).width;
		area  = 1.0 / (_xitr * _yitr);
		_y    = 0;
		_x    = 0;
		int tf = video.get(cv::CAP_PROP_FRAME_COUNT);
		int cf = 0;
		
		clock_t time;
		double  waittime = 0.0;
		int		yend	 = video_height - _yitr;
		int		xend	 = video_width - _xitr;

		printf("Processing...\n");
		while (video.read(_frm))
		{
			printf("Processed Frames: %d/%d\t\tEstimated wait time: %.3fs (~%1.0fmin)\r", cf, tf, waittime, waittime / 60);

			time = clock();

			cv::Mat newmat(
				size,
				CV_8UC1,
				cv::Scalar(0, 0, 0)
			);

			for (_y = 0; _y < yend; _y = _y + _yitr) {
			for (_x = 0; _x < xend; _x = _x + _xitr) {

				char* _simchrs;
				int* _caverage;
				GetAverageColor(&_caverage);								// Colored average

				// Grayscale average
				int   _gaverage = grayscale_conversion_type(_caverage[0], _caverage[1], _caverage[2]);
				char  _chr[2]    = { ' ', '\0' };

				/*	---				CONVERSION BEGINS HERE				---	*/

				if (variation != Variations::STANDARD) goto skipstandard;	// Tests if variation is STANDARD
				if (GetRand(1, 50) != 1) goto skipstandard;					// Draws random number. Skip STANDARD if the number is 1

				_chr[0] = (char)GetRand(STARTING_CHAR, ENDING_CHAR);		// Sets the chosen character to a random character
				goto convert_loop_end;										// Goes to the end of the loop
			
			// Go here if the variation is not STANDARD or the number generator failed to give 1.
			skipstandard:
				GetSimilarCharacters(_gaverage, spectrum, threshold, &_simchrs);

				// Adding 1 to threshold prevents _min from being 0 and _max being 1(GetRand breaks when _min = 0 and _max = 1)
				// Subtract the resulting number by 1 to get the actual drawn number
				_chr[0] = _simchrs[GetRand(1, threshold) - 1];

			// End of for loop
			convert_loop_end:
				ft2->putText(newmat, _chr, cv::Point(_x, _y), _yitr, (color) ? cv::Scalar(_caverage[2], _caverage[1], _caverage[0]) : _gaverage, -1, cv::LINE_AA, false);
			}}

			result.write(newmat);
			cf++;

			time = clock() - time;

			// Calculate estimated wait time
			waittime = (((double)time) / CLOCKS_PER_SEC) * (tf - cf);
		}
		printf("Processed Frames: %d/%d\tEstimated wait time: 0s (0min)\n", cf, tf);

		video.release();
		result.release();
		
		printf("Completed.\n");
	}
	
	void calibrate(bool overwrite_current_spectrum, bool overwrite_mapfile, double bias, GrayScaleType grayscale_conversion_type)
	{
		printf("Copying files... \n");
		FILE* file;
		FILE* newfile;
		FILE* tmpfile;
		char  tmptext[BUFFER_FILE_SIZE];

		// Opens files and checks if they are successful
		if (!(file = fopen(mapfile, "r"))) throw _statexception("Cannot open mapfile in calibration function");
		if (!(tmpfile = fopen(TEMP_FILE_NAME, "w"))) throw _statexception("Cannot write tmpfile in calibration function");

		// Copies the comments from mapfile into tmpfile
		while (fgets(tmptext, BUFFER_FILE_SIZE, file)) { if (tmptext[1] == '/' && tmptext[0] == '/') fputs(tmptext, tmpfile); }

		fclose(file);
		fclose(tmpfile);

		// Opens tmpfile as read and writes newfile
		// newfile is mapfile.txt if overwrite_mapfile is true, otherwise it creates its own file
		if (!(tmpfile = fopen(TEMP_FILE_NAME, "r"))) throw _statexception("Cannot open tmp file in calibration function");
		if (!(newfile = fopen(
			(overwrite_mapfile) ? mapfile : NEW_FILE_NAME,
			"w"
		))) throw _statexception("Cannot erase current or new mapfile");

		// Above code opens it as write to erase all data in it
		// So, close old newfile and reopen it in "a" mode
		fclose(newfile);
		if (!(newfile = fopen(
			(overwrite_mapfile) ? mapfile : NEW_FILE_NAME,
			"a"
		))) throw _statexception("Cannot write new mapfile or overwrite current mapfile");

		// Copies contents of tmp file to newfile
		while (fgets(tmptext, BUFFER_FILE_SIZE, tmpfile)) { fputs(tmptext, newfile); }
		fclose(tmpfile);
		remove(TEMP_FILE_NAME);

		fputc('\n', newfile);		// Ensures a new line is present

		
		printf("Beginning calibration...\n");

		cv::Scalar basecolor(255, 255, 255);
		cv::Scalar textcolor(0, 0, 0);
		cv::Point  origin(0, 0);
		cv::Size   size = ft2->getTextSize(DEFAULT_CHARACTER, CALIBRATION_FONT_SIZE, -1, 0);
		// Needs to be set, otherwise GetAverageColor() won't work
		_yitr = size.height;
		_xitr = size.width;
		area  = 1.0 / (_xitr * _yitr);

		for (char subject[2]{ STARTING_CHAR, '\0' }; subject[0] <= ENDING_CHAR; subject[0]++)
		{
			//printf("Character: %c\r", subject[0]);
			int*    _cav;			// Average color
			int     _gav = 255;		// Average grayscale
			if (subject[0] == ' ') goto saveaverage;


			_frm = cv::Mat(size, CV_8UC1, basecolor);
			ft2->putText(_frm, subject, origin, CALIBRATION_FONT_SIZE, textcolor, -1, cv::LINE_AA, false);
			_y = 0;
			_x = 0;


			GetAverageColor(&_cav);
			_gav = bias * grayscale_conversion_type(_cav[0], _cav[1], _cav[2]);

		saveaverage:					// Label because I can ;)
			if (overwrite_current_spectrum) spectrum[subject[0] - STARTING_CHAR] = _gav;

			// Writes the first part of the line
			putc(subject[0], newfile);
			putc(':', newfile);

			// Converts average to cstring and writes to file
			char c_average[3];
			sprintf(c_average, "%d", _gav);
			fputs(c_average, newfile);

			// Newline
			putc('\n', newfile);
		}
		printf("Character: %c\n", (char)ENDING_CHAR);
		printf("Calibration completed.\n");

		fclose(newfile);
	}
private:
	cv::Ptr<cv::freetype::FreeType2> ft2 = cv::freetype::createFreeType2();	// Freetype font
	cv::Mat _frm;		// Extracted frame
	int _y;				// Current y-coordinate
	int _x;				// Current x-coordinate
	int _yitr;			// Iteration height
	int _xitr;			// Iteration width
	char* mapfile = (char*)"";
	double area;		// Thing that is used in GetAverageColor()
	// Characters to use in conversion -- character spectrum
	// Structure:
	// [
	// [min, max],
	// [min, max],
	// ...
	// ]
	// * INDEX CORRESPONDS TO:
	//	CHARACTER DECIMAL - STARTING_CHAR
	int spectrum[ENDING_CHAR - STARTING_CHAR];

#define TEST(obj, end) if(obj == NULL) goto end
	void StripString(char* string)
	{
		if (!strlen(string)) return;

		char* result = (char*)malloc(sizeof(char));
		int   length = strlen(string);
		int   c = 1;
		TEST(result, stripstringend);
		result[0] = '\0';

		for (int i = 0; i < length; i++)
		{
			char subject = string[i];
			if (subject == '\t' || subject == ' ' || subject == '\n') continue;

			// Allocate new memory
			char* tmp = (char*)realloc(result, c * sizeof(result));
			TEST(tmp, stripstringend);
			result = tmp;

			// Append characters
			result[c - 1] = subject;
			result[c] = '\0';
			c++;
		}

		// Using labels because I can't be fucked to exit while in the loop
	stripstringend:
		string = result;
		free(result);
	}

	void CleanSpectrum(char* string, int* out)
	{
		char* result = (char*)malloc(sizeof(char));
		bool  atcolon = false;
		int	  length = strlen(string);
		int   c = 1;
		TEST(result, cleanspectrumend);

		for (int i = 2; i < length; i++)
		{
			char subject = string[i];

			char* tmp = (char*)realloc(result, c * sizeof(char));
			TEST(tmp, cleanspectrumend);
			result = tmp;

			result[c - 1] = subject;
			c++;
		}

	cleanspectrumend:
		(*out) = atoi(result);
		free(result);
	}

	void GetSimilarCharacters(int _color, int* _spectrum, int _threshold, char** _buffer)
	{
		int* priority = (int*)malloc(_threshold * sizeof(int) * 2);	// Dynamic 2d array. Column 0 is the spectrum value, column 1 is the character
		int  max = ENDING_CHAR - STARTING_CHAR;
		TEST(priority, simcharsend);

		// i = current char - STARTING_CHAR
		for (int i = 0; i <= max; i++)
		{
			/*						PUSHES HIGHEST SPECTRUM INTO priority							*/
			/*			ALSO KEEPS TRACK OF CHARACTERS CORRESPONDANT TO THAT PRIORITY				*/

			int prv_idx[2]{};										// Stores previous column value
			int mov_idx[2]{ -1 };										// Column 0 and 1 value. mov_idx[0] being -1 means the array is empty

			for (int j = 0; j < _threshold; j++)
			{
				int col1 = j * 2 + 1;								// column 1 of priority's jth index
				int col0 = j * 2;									// column 0 of priority's jth index

				if (mov_idx[0] == -1) goto exchangelabel;			// If mov_idx[0] is empty then skip to exchangelabel

				// Suffles numbers down

				prv_idx[1] = priority[col1];						// Save to-be-overwritten row
				prv_idx[0] = priority[col0];

				priority[col1] = mov_idx[1];						// Overwrite row from handedoff row
				priority[col0] = mov_idx[0];

				mov_idx[1] = prv_idx[1];							// Handoff stored row
				mov_idx[0] = prv_idx[0];

				continue;

			exchangelabel:

				// DOES NOT continue if val_diff IS NOT LESS THAN the absolute difference of _val minus priority index j colum 0
				// ONLY continue when val_diff is LESS THAN whatever it is being compared to

				int _val = _spectrum[i];
				if (abs(_val - _color) > abs(_val - priority[col0])) break;

				prv_idx[1] = priority[col1];						// Save to-be-overwritten row
				prv_idx[0] = priority[col0];

				priority[col1] = i;									// Overwrite row
				priority[col0] = _val;

				mov_idx[1] = prv_idx[1];							// Handoff prv_idx
				mov_idx[0] = prv_idx[0];
			}
		}

	simcharsend:
		char* tmp = (char*)malloc(_threshold * sizeof(char));
		TEST(tmp, simcharsend2);

		for (int i = 0; i < _threshold; i++) {
			tmp[i] = (char)(priority[i * 2 + 1] + STARTING_CHAR);
		}

	simcharsend2:
		(*_buffer) = tmp;

		free(priority);
	}

	int GetRand(int _min, int _max) { return _min + rand() % (_max - _min); }
	// Returns dynamic array with three indexes:
	// 0 = R
	// 1 = G
	// 2 = B
	void GetAverageColor(int** _buffer)
	{
		int*   average = (int*)malloc(3 * sizeof(int));	// RGB memory
		int    _r      = 0;
		int    _g      = 0;
		int    _b      = 0;
		int    yend    = _y + _yitr;
		int    xend	   = _x + _xitr;
		TEST(average, averagecolorend);

		for (int y = _y; y <= yend; y++) {
		for (int x = _x; x <= xend; x++) {
			cv::Vec3b color = _frm.at<cv::Vec3b>(y, x);

			//average[0] += color[2];						// Adds R value
			_r += color[2];
			//average[1] += color[1];						// Adds G value
			_g += color[1];
			//average[2] += color[0];						// Adds B value
			_b += color[0];
		}}
		
		_r *= area;
		_g *= area;
		_b *= area;

		average[0] = _r;									// Divides sums by the area
		average[1] = _g;
		average[2] = _b;

	averagecolorend:
		(*_buffer) = average;
		free(average);
	}
};