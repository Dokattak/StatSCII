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
		size = cv::Size(video_width, video_height);
		


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
		yend = video_height - _yitr;
		xend = video_width - _xitr;



		if (color) goto skipsetup;
		BeginSetup();
		
		/*															*/
		/* ---				STATSCII BEGINS HERE				---	*/
		/*															*/

		printf("Character: %c\n", ENDING_CHAR);
		
	skipsetup:

		printf("Processing...\n");
		while (video.read(_frm))
		{
			printf("Processed Frames: %d/%d\t\tEstimated wait time: %.3fs (~%1.1fmin)                   \r", cf, tf, waittime, waittime / 60);

			time = clock();
			
			cv::Mat newmat = convertimage(_frm);
			result.write(newmat);
			cf++;

			time = clock() - time;
			// Calculate estimated wait time
			waittime = (((double)time) / CLOCKS_PER_SEC) * (tf - cf);
		}
		printf("Processed Frames: %d/%d\tEstimated wait time: 0s (0min)\n", cf, tf);
		
		video.release();
		result.release();

		printf("Cleaning temporary setup files...\n");
		
		/*											 */
		/* ---	   Deletes Setup Directory		 --- */
		/*											 */

		WIN32_FIND_DATAA _fd;
		HANDLE _hf = FindFirstFileA("setup/*.*", &_fd);
		char _fn[1024];

		do {
			sprintf(_fn, "setup/%s", _fd.cFileName);
			DeleteFileA(_fn);
		} while (FindNextFileA(_hf, &_fd));

		FindClose(_hf);
		if(_rmdir("setup/")) printf("Error: could not remove setup directory\n");
		
		printf("Completed.\n");
	}
	
	cv::Mat convertimage(char* _filepath, int height, int save_as_file)
	{
		cv::Mat source = cv::imread(_filepath);
		
		_yitr = source.rows / height;
		_xitr = ft2->getTextSize(DEFAULT_CHARACTER, _yitr, -1, 0).width;
		yend  = source.rows - _yitr;
		xend  = source.cols - _xitr;
		area  = 1.0 / (_xitr * _yitr);
		size  = cv::Size(source.cols, source.rows);

		BeginSetup();

		cv::Mat newmat = convertimage(source);
		if (save_as_file) cv::imwrite("result.png", newmat);

		return newmat;
	}
	cv::Mat convertimage(cv::Mat _image)
	{
		_frm = _image;
		cv::Mat out(size, CV_8UC4, bgcolor);

		for (_y = 0; _y < yend; _y = _y + _yitr) {
			for (_x = 0; _x < xend; _x = _x + _xitr) {

				char* _simchrs;
				int* _caverage;
				GetAverageColor(&_caverage);										// Colored average

				if (invert_color) InvertColors(&_caverage);							// Invert colors

				// Grayscale average
				int  _gaverage = gstype(_caverage[0], _caverage[1], _caverage[2]);
				char _chr;

				/*															*/
				/*	---				CONVERSION BEGINS HERE				---	*/
				/*															*/

				if (!apply_static) goto skipstandard;								// Skips static application if apply_static is false
				if (GetRand(1, _frm.rows * _frm.cols * 2) != 1) goto skipstandard;	// Draws random number. Skip STANDARD if the number is 1

				_chr = (char)GetRand(STARTING_CHAR, ENDING_CHAR);					// Sets the chosen character to a random character
				goto convert_loop_end;												// Goes to the end of the loop

				// Go here if the variation is not STANDARD or the number generator failed to give 1.
			skipstandard:

				GetSimilarCharacters(_gaverage, &_simchrs);

				// Get random similar character
				_chr = _simchrs[GetRand(0, threshold - 1)];

				// End of for loop
			convert_loop_end:

				if (_chr == ' ') continue;
				if (color)
				{
					if (invert_color) InvertColors(&_caverage);

					ft2->putText(
						out, { _chr, '\0' },
						cv::Point(_x, _y), _yitr,
						cv::Scalar(_caverage[2], _caverage[1], _caverage[0]),
						-1, cv::LINE_AA, false
					);
					continue;
				}

				char* _path;
				GetSetupImageName(_chr, &_path);

				cv::Mat _chm = cv::imread(_path, cv::IMREAD_UNCHANGED);
				PutMatOnMat(_chm, out);

				
			}
		}

		return out;
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
			printf("Character: %c\r", subject[0]);
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

	cv::Scalar textcolor	= cv::Scalar(255, 255, 255, 255);
	cv::Scalar bgcolor		= cv::Scalar(0, 0, 0, 255);
	GSType	   gstype		= GSTypes::RelativeLuminance;
	bool	   color		= 0;
	bool	   apply_static	= 1;
	bool       invert_color = 1;
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

	cv::Size size;
	int yend;
	int xend;

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
	void PutMatOnMat(cv::Mat& _in, cv::Mat& _out)
	{
		int _yend = _y + _in.rows;
		int _xend = _x + _in.cols;

		if (_yend > _out.rows) _yend = _out.rows;
		if (_xend > _out.cols) _xend = _out.cols;

		for (int y = _y; y < _yend; y++) {
		for (int x = _x; x < _xend; x++) {

			cv::Vec4b _v = _in.at<cv::Vec4b>(y - _y, x - _x);
			if (_v[3] == 0) continue;

			_out.at<cv::Vec4b>(y, x) = _v;

		}}
	}

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
		sprintf(_charnum, "%d", _num);
		if (_num < 100)					// If the number is less than 100, then index 0 needs to be a 0
		{
			_charnum[2] = _charnum[1];
			_charnum[1] = _charnum[0];
			_charnum[0] = '0';
		}

		char _res[]{ 's', 'e', 't', 'u', 'p', '/', _charnum[0], _charnum[1], _charnum[2], '.', 'p', 'n', 'g' , '\0' };
		(*_buffer) = _res;
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
			priority[i * 2]     = -1;
		}


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
				
				if (priority[col0] == -1) goto skiptestlabel;
				if (abs(_color - _val) > abs(_color - priority[col0])) break;
				
			skiptestlabel:

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
		int* _av    = (int*)malloc(3 * sizeof(int));
		int  _yend  = _y + _yitr;
		int  _xend	= _x + _xitr;
		TEST(_av, averagecolorend);

		// Sets initial values to 0
		for (int i = 0; i < 3; i++) { _av[i] = 0; }

		for (int y = _y; y < _yend; y++) {
		for (int x = _x; x < _xend; x++) {

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

	void InvertColors(int** _in)
	{
		(*_in)[0] = 255 - (*_in)[0];
		(*_in)[1] = 255 - (*_in)[1];
		(*_in)[2] = 255 - (*_in)[2];
	}

	void BeginSetup()
	{
		printf("Beginning setup...\n");

		// Attempt to make directory. _mkdir returns 0 if succeeded
		int _res = _mkdir("setup");
		// If the directory hasn't been created, and _res failed, then an error occured
		if (!(GetFileAttributesA("setup/") & INVALID_FILE_ATTRIBUTES) && _res) throw _statexception("Cannot create new directory for setup");


		for (char chr[2] = { STARTING_CHAR, '\0' }; chr[0] <= ENDING_CHAR; chr[0]++)
		{
			
			printf("Character: %c\r", chr[0]);

			cv::Mat _m(cv::Size(_xitr * 1.10, _yitr * 1.30), CV_8UC4, cv::Scalar(0, 0, 0, 0));
			ft2->putText(_m, chr, _CAL_origin, _yitr, textcolor, -1, cv::LINE_AA, false);

			char* _path;
			GetSetupImageName(chr[0], &_path);
			cv::imwrite(_path, _m);
		}
	}
};