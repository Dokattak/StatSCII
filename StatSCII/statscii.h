#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <windows.h>
#include <stdlib.h>
#include <direct.h>

#include <iostream>
#include <random>
#include <ctime>

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

	const cv::Scalar _CAL_basecolor(255, 255, 255);
	const cv::Scalar _CAL_textcolor(0, 0, 0);
	const cv::Point  _CAL_origin(0, 0);
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

typedef int (*GSType)(int, int, int);
namespace GSTypes
{
	/* ---				Grayscale Functions				--- */
	int RelativeLuminance(int _r, int _g, int _b) { return 0.2126 * _r + 0.7152 * _g + 0.0722 * _b; }
	int Lightness(int _r, int _g, int _b) { int n[3] = { _r, _g, _b }; return GetMax(n, 3) * 0.5 + GetMin(n, 3) * 0.5; }
	int Contrast(int _r, int _g, int _b) { return 0.299 * _r + 0.587 * _g + 0.114 * _b; }
	int Average(int _r, int _g, int _b) { return _r * _onethird + _g * _onethird + _b * _onethird; }
};

struct statscii
{
	statscii(char* fontfile, char* mapfile) : _mapfile(mapfile)
	{
		srand(time(0));

		// Sets font
		ft2->loadFontData(fontfile, 0);

		FILE* file;								// Character map file
		char  text[BUFFER_FILE_SIZE];				// Line of file

		// If mapfile is null throw esception
		if (!(file = fopen(_mapfile, "r"))) throw _statexception("Cannot open character map file");

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

	void convert(char* videofile, int height = 50)
	{
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
		int tf = video.get(cv::CAP_PROP_FRAME_COUNT);
		int cf = 0;

		clock_t time;
		double  waittime = 0.0;
		int		yend	 = video_height - _yitr;
		int		xend	 = video_width - _xitr;

		/*															*/
		/* ---				SETUP BEGINS HERE					---	*/
		/*															*/

		char _path[] = "setup/ .png\0";
		int _res;
		if (color) goto skipsetup;
		printf("Beginning setup...\n");


		// Attempt to make directory. _mkdir returns 0 if succeeded
		_res = _mkdir("setup");
		// If the directory hasn't been created, and _res failed, then an error occured
		if (!(GetFileAttributesA("setup/") & INVALID_FILE_ATTRIBUTES) && _res) throw _statexception("Cannot create new directory for setup");
		

		for (char chr[2] = { STARTING_CHAR, '\0' }; chr[0] <= ENDING_CHAR; chr[0]++)
		{
			printf("Character: %c\r", chr[0]);

			cv::Mat _m = cv::Mat(cv::Size(_xitr + 10, _yitr+ 20), CV_8UC1, bgcolor);
			ft2->putText(_m, chr, _CAL_origin, _yitr, textcolor, -1, cv::LINE_AA, false);

			_path[6] = chr[0];
			cv::imwrite(_path, _m);
		}
		return;
		/*															*/
		/* ---				STATSCII BEGINS HERE				---	*/
		/*															*/

		printf("Character: %c\n", ENDING_CHAR);

	skipsetup:

		printf("Processing...\n");
		while (video.read(_frm))
		{
			printf("Processed Frames: %d/%d\t\tEstimated wait time: %.3fs (~%1.0fmin)\r", cf, tf, waittime, waittime / 60);

			time = clock();

			cv::Mat newmat(size, CV_8UC1, bgcolor);

			for (_y = 0; _y < yend; _y = _y + _yitr) {
			for (_x = 0; _x < xend; _x = _x + _xitr) {
				
				char* _simchrs;
				int* _caverage;
				GetAverageColor(&_caverage);								// Colored average
				
				// Grayscale average
				int   _gaverage = gstype(_caverage[0], _caverage[1], _caverage[2]);
				char  _chr;

				/*															*/
				/*	---				CONVERSION BEGINS HERE				---	*/
				/*															*/

				if (!apply_static) goto skipstandard;						// Skips static application if apply_static is false
				if (GetRand(1, 50) != 1) goto skipstandard;					// Draws random number. Skip STANDARD if the number is 1

				_chr = (char)GetRand(STARTING_CHAR, ENDING_CHAR);			// Sets the chosen character to a random character
				goto convert_loop_end;										// Goes to the end of the loop
			
			// Go here if the variation is not STANDARD or the number generator failed to give 1.
			skipstandard:

				GetSimilarCharacters(_gaverage, &_simchrs);
				
				// Get random similar character
				_chr = _simchrs[GetRand(0, threshold - 1)];
				
			// End of for loop
			convert_loop_end:

				_path[6] = _chr;
				DWORD _chrexist = GetFileAttributesA(_path);

				if (_chr == ' ') continue;
				if (color || _chrexist == INVALID_FILE_ATTRIBUTES)	// This is a last resort if the character wasn't found or if color is false
				{
					ft2->putText(
						newmat, { _chr, '\0' },
						cv::Point(_x, _y), _yitr,
						(color) ? cv::Scalar(_caverage[2], _caverage[1], _caverage[0]) : textcolor,
						-1, cv::LINE_AA, false
					);
					continue;
				}

				cv::Mat _chm = cv::imread(_path, cv::IMREAD_UNCHANGED);

				// FIND A BETTER COPYING METHOD!!!!!
				_chm.copyTo(newmat(cv::Rect(_x, _y, _chm.cols, _chm.rows)));
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
		RemoveDirectoryA(_path);
		
		printf("Completed.\n");
	}
	
	void calibrate(bool overwrite_current_spectrum, bool overwrite_mapfile, double bias, GSType grayscale_conversion_type)
	{
		printf("Copying files... \n");
		FILE* file;
		FILE* newfile;
		FILE* tmpfile;
		char  tmptext[BUFFER_FILE_SIZE];

		// Opens files and checks if they are successful
		if (!(file = fopen(_mapfile, "r"))) throw _statexception("Cannot open mapfile in calibration function");
		if (!(tmpfile = fopen(TEMP_FILE_NAME, "w"))) throw _statexception("Cannot write tmpfile in calibration function");

		// Copies the comments from _mapfile into tmpfile
		while (fgets(tmptext, BUFFER_FILE_SIZE, file)) { if (tmptext[1] == '/' && tmptext[0] == '/') fputs(tmptext, tmpfile); }

		fclose(file);
		fclose(tmpfile);

		// Opens tmpfile as read and writes newfile
		// newfile is mapfile.txt if overwrite_mapfile is true, otherwise it creates its own file
		if (!(tmpfile = fopen(TEMP_FILE_NAME, "r"))) throw _statexception("Cannot open tmp file in calibration function");
		if (!(newfile = fopen(
			(overwrite_mapfile) ? _mapfile : NEW_FILE_NAME,
			"w"
		))) throw _statexception("Cannot erase current or new mapfile");

		// Above code opens it as write to erase all data in it
		// So, close old newfile and reopen it in "a" mode
		fclose(newfile);
		if (!(newfile = fopen(
			(overwrite_mapfile) ? _mapfile : NEW_FILE_NAME,
			"a"
		))) throw _statexception("Cannot write new mapfile or overwrite current mapfile");

		// Copies contents of tmp file to newfile
		while (fgets(tmptext, BUFFER_FILE_SIZE, tmpfile)) { fputs(tmptext, newfile); }
		fclose(tmpfile);
		remove(TEMP_FILE_NAME);

		fputc('\n', newfile);		// Ensures a new line is present

		
		printf("Beginning calibration...\n");

		cv::Size size = ft2->getTextSize(DEFAULT_CHARACTER, CALIBRATION_FONT_SIZE, -1, 0);
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


			_frm = cv::Mat(size, CV_8UC1, _CAL_basecolor);
			ft2->putText(_frm, subject, _CAL_origin, CALIBRATION_FONT_SIZE, _CAL_textcolor, -1, cv::LINE_AA, false);
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

	/*																*/
	/* ---				Configuration Variables					--- */
	/*																*/

	cv::Scalar textcolor	= cv::Scalar(255, 255, 255);
	cv::Scalar bgcolor		= cv::Scalar(0, 0, 0);
	GSType	   gstype		= GSTypes::RelativeLuminance;
	bool	   color		= 0;
	bool	   apply_static	= 1;
	int		   threshold	= 1;

private:
	cv::Ptr<cv::freetype::FreeType2> ft2 = cv::freetype::createFreeType2();	// Freetype font
	cv::Mat _frm;		// Extracted frame
	int _y;				// Current y-coordinate
	int _x;				// Current x-coordinate
	int _yitr;			// Iteration height
	int _xitr;			// Iteration width
	char* _mapfile = (char*)"";
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
	void StripString(char* _string)
	{
		if (!strlen(_string)) return;

		char* result = (char*)malloc(sizeof(char));
		int   length = strlen(_string);
		int   c = 1;
		TEST(result, stripstringend);
		result[0] = '\0';

		for (int i = 0; i < length; i++)
		{
			char subject = _string[i];
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
		_string = result;
		free(result);
	}

	void CleanSpectrum(char* _string, int* _buffer)
	{
		char* result = (char*)malloc(sizeof(char));
		bool  atcolon = false;
		int	  length = strlen(_string);
		int   c = 1;
		TEST(result, cleanspectrumend);

		for (int i = 2; i < length; i++)
		{
			char subject = _string[i];

			char* tmp = (char*)realloc(result, c * sizeof(char));
			TEST(tmp, cleanspectrumend);
			result = tmp;

			result[c - 1] = subject;
			c++;
		}

	cleanspectrumend:
		(*_buffer) = atoi(result);
		free(result);
	}

	void GetSetupImageName(int _num, char** _buffer)
	{
		char _charnum[3];
		char _su[7]{ "setup/" };
		char _pg[5]{ ".png" };
		sprintf(_charnum, "%d", _num);

		// Plus 1 for string terminator
		char* _res = (char*)malloc((15 + 1) * sizeof(char));
		TEST(_res, setupimagenameend);

		// Figure out way to concatenate _su, _charnum, and _pg together in that order
		// Remember to account for any extra 0's in _charnum

	setupimagenameend:
		(*_buffer) = _res;
		free(_res);
	}

	void GetSimilarCharacters(int _color, char** _buffer)
	{
		int* priority = (int*)malloc(threshold * sizeof(int) * 2);	// Dynamic 2d array. Column 0 is the spectrum value, column 1 is the character
		int  max = ENDING_CHAR - STARTING_CHAR;
		TEST(priority, simcharsend);

		// Sets initial values to 0
		for (int i = 0; i < threshold; i++)
		{
			priority[i * 2 + 1] = 0;
			priority[i * 2]     = 0;
		}

		//printf("[%d,%d],[%d,%d]\n", priority[0 * 2 + 0], priority[0 * 2 + 1], priority[1 * 2 + 0], priority[1 * 2 + 1]);
		// i = current char - STARTING_CHAR
		for (int i = 0; i <= max; i++)
		{

			/*																						*/
			/*						PUSHES HIGHEST SPECTRUM INTO priority							*/
			/*			ALSO KEEPS TRACK OF CHARACTERS CORRESPONDANT TO THAT PRIORITY				*/
			/*																						*/

			int prv_idx[2]{};										// Stores previous column value
			int mov_idx[2]{ -1 };									// Column 0 and 1 value. mov_idx[0] being -1 means the array is empty

			for (int j = 0; j < threshold; j++)
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

				int _val = spectrum[i];
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
		char* tmp = (char*)malloc(threshold * sizeof(char));
		TEST(tmp, simcharsend2);

		for (int i = 0; i < threshold; i++) {
			tmp[i] = (char)(priority[i * 2 + 1] + STARTING_CHAR);
		}

	simcharsend2:
		(*_buffer) = tmp;
		free(tmp);
		free(priority);
	}

	int GetRand(int _min, int _max)
	{
		if (_min == _max) return _min;
		if (_max - _min == 1) return _min + rand() % 2;

		return _min + rand() % (_max - _min);
	}
	// Returns dynamic array with three indexes:
	// 0 = R
	// 1 = G
	// 2 = B
	void GetAverageColor(int** _buffer)
	{
		int* _av = (int*)malloc(3 * sizeof(int));
		int    yend    = _y + _yitr;
		int    xend	   = _x + _xitr;
		TEST(_av, averagecolorend);

		// Sets initial values to 0
		for (int i = 0; i < 3; i++) { _av[i] = 0; }

		for (int y = _y; y <= yend; y++) {
		for (int x = _x; x <= xend; x++) {

			cv::Vec3b color = _frm.at<cv::Vec3b>(y, x);
			_av[0] += color[2];							// Adds R value
			_av[1] += color[1];							// Adds B value
			_av[2] += color[0];							// Adds G value

		}}
		
		_av[0] *= area;									// Divides sums by the area
		_av[1] *= area;									// Technically the area is stored as a decimal; as 1.0 / ( w * h )
		_av[2] *= area;

	averagecolorend:
		(*_buffer) = _av;
		free(_av);
	}
};