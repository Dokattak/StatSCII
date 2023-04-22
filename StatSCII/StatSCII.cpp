#include <iostream>
#include "statscii.h"

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cout <<
			"StatSCII Help\n\n" <<
			"Format: statscii [arg] [val]\n\n" <<
			"Arguements:\n" <<
			"\t[name] = [datatype]; [description]\n" <<
			"\t[-src] = File Path; Source of video file to be converted\n" <<
			"\t[-fnt] = File Path; Source of font file\n" <<
			"\t[-spf] = File Path; Source of custom map file(OPTIONAL)\n" <<
			"\t[-chh] = Integer; Height of resulting video in characters\n\n" <<
			"\t[-thr] = Integer; Number of similar characters gathered before insertion\n" <<
			"\t[-col] = None; If present, the resulting characters will inherit the average color that they took up\n" <<
			"\t[-gcv] = Integer; Number corresponding to the type of RGB to grayscale to use. Possible values:\n" <<
			"\t\tRelative Luminance = 0\n" <<
			"\t\tLightness = 1\n" <<
			"\t\tContrast = 2\n" <<
			"\t\tAverage = 3\n" <<
			"\t[-stc] = None; If present, StatSCII will apply its 'ASCII Static' to the resulting video\n" <<
			"Special Commands:\n" <<
			"\t[-cal] = None; Calibrates a map file and outputs it as new_mapfile.txt\n" <<
			"\tNOTE: The [-cal] arguement can be used with any of the following arguements:\n" <<
			"\t\t[-bis] = Decimal; The bias applied to the average value when calibrated. Can be any number with or without a decimal.\n" <<
			"\t\t[-overmap] = None; Overwrites mapfile.txt instead of making a new file\n" <<
			std::endl;
		return 0;
	}

	GSType grayscale_conversion_type = GSTypes::RelativeLuminance;
	double bias						 = 1;
	char*  videofile				 = (char*)"";
	char*  fontfile					 = (char*)"defaultfont.ttf";
	char*  specfile					 = (char*)"mapfile.txt";
	bool   calibrate				 = false;
	bool   overwrite_mapfile		 = false;
	bool   wantcolor				 = false;
	bool   wantstatic				 = false;
	int    threshold				 = 1;
	int    height					 = 20;

	for (int i = 1; i < argc; i = i + 2)
	{
		char* prm = argv[i + 1];
		char* arg = argv[i];

		if (strcmp(arg, "-src") == 0) videofile = prm;
		else if (strcmp(arg, "-fnt") == 0) fontfile = prm;
		else if (strcmp(arg, "-spf") == 0) specfile = prm;
		else if (strcmp(arg, "-chh") == 0) height = atoi(prm);
		else if (strcmp(arg, "-col") == 0) { wantcolor = true; i--; }
		else if (strcmp(arg, "-thr") == 0) threshold = atoi(prm);
		else if (strcmp(arg, "-gcv") == 0)
		{
			int x = atoi(prm);
			switch (x)
			{
			case 0: { grayscale_conversion_type = GSTypes::RelativeLuminance; continue; }
			case 1: { grayscale_conversion_type = GSTypes::Lightness; continue; }
			case 2: { grayscale_conversion_type = GSTypes::Contrast; continue; }
			case 3: { grayscale_conversion_type = GSTypes::Average; continue; }
			}
			printf("Invalid GrayScaleConversion type '%d'\n", x);
			return -1;
		}
		else if (strcmp(arg, "-stc") == 0) { wantstatic = true; i--; }
		else if (strcmp(arg, "-cal") == 0) { calibrate = true; i--; }
		else if (strcmp(arg, "-overmap") == 0) { overwrite_mapfile = true; i--; }
		else if (strcmp(arg, "-bis") == 0) sscanf(prm, "%lf", &bias);
		else
		{
			printf("Invalid Arguement '%s'\n", arg);
			return -1;
		}
	}

	statscii stcii(fontfile, specfile);
	
	try
	{
		stcii.apply_static = wantstatic;
		stcii.color		   = wantcolor;
		stcii.gstype	   = grayscale_conversion_type;
		stcii.threshold    = threshold;

		if (calibrate) stcii.calibrate(false, overwrite_mapfile, bias, grayscale_conversion_type);
		if (videofile == "")
		{
			printf("Error: source video was not defined. Exiting...");
			return -1;
		}
		stcii.convert(videofile, height);
	}
	catch (std::exception& e) { printf(e.what()); }

	printf("Program end\n");
	return 0;
}