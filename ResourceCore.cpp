#include "ResourceCore.h"
#include <stdio.h>
#include <png.h>
#include <vector>
///////////////////////////// Guard classes.  Makes loading structures exception safe.

class FileGuard{
	FILE *myPointer;
public:
	FileGuard(FILE *ToGuard = 0) : myPointer(ToGuard){}
	~FileGuard(){ if(myPointer) fclose(myPointer); }
	void GuardFile(FILE* ToGuard){ myPointer = ToGuard; }
	void StopGuarding(){ myPointer = 0; }
};

class SurfaceGuard{
	SDL_Surface* myPointer;
public:
	SurfaceGuard(SDL_Surface *ToGuard = 0) : myPointer(ToGuard){}
	~SurfaceGuard(){ if(myPointer) SDL_FreeSurface(myPointer); }
	void GuardSurface(SDL_Surface* ToGuard){ myPointer = ToGuard; }
	void StopGuarding(){ myPointer = 0; }
};

class PngGuard{
	png_struct *PngStruct;
	png_info *StartInfo;
	png_info *EndInfo;
public:
	PngGuard(png_struct *ToGuard = 0) : PngStruct(ToGuard), StartInfo(0), EndInfo(0){}
	~PngGuard(){ if(PngStruct || StartInfo || EndInfo) png_destroy_read_struct(&PngStruct,&StartInfo,&EndInfo); }
	void GuardPngStruct(png_struct* ToGuard){ PngStruct = ToGuard; }
	void GuardStartInfo(png_info* ToGuard){ StartInfo = ToGuard; }
	void GuardEndInfo(png_info* ToGuard){ EndInfo = ToGuard; }
	void StopGuarding(){ PngStruct = 0; StartInfo = 0; EndInfo = 0; }
};

SDL_Surface* LoadPng(const char* Filename){
	//Prepare the guards - prepare here so all are called by lngjmp
	FileGuard guardfin;
	PngGuard guardPng;
	SurfaceGuard guardRetVal;

	//Open the file
	FILE *fin = fopen(Filename, "rb");
	guardfin.GuardFile(fin);
	if (!fin)
		return 0;

	//Verify the header
	{
		png_byte Header[8];
		fread(Header,1,8,fin);
		if(!png_sig_cmp(Header,0,8))
			return 0; // File closing handled by guard.
	}
	//Initialize structures
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	guardPng.GuardPngStruct(png_ptr);
	if (!png_ptr)
		return 0; // File closing handled by guard
	png_infop info_ptr = png_create_info_struct(png_ptr);
	guardPng.GuardStartInfo(info_ptr);
	if (!info_ptr)
		return 0; // File and png closing handled by guards
	png_infop end_info = png_create_info_struct(png_ptr);
	guardPng.GuardEndInfo(info_ptr);
	if (!end_info)
		return 0; // File and png closing handled by guards
	if (setjmp(png_jmpbuf(png_ptr)))
		return 0; // If there's an error, jump here.  File and png closing still handled by guards

	// Read the png header
	png_init_io(png_ptr,fin);
	png_set_sig_bytes(png_ptr,8);
	unsigned int width = png_get_image_width(png_ptr, info_ptr);
	unsigned int height = png_get_image_height(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);

	// Modify so pixel data is RGBA
	if (color_type == PNG_COLOR_TYPE_PALETTE) // palette -> rgb
		png_set_palette_to_rgb(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) // grayscale 2,4 -> rgba
		png_set_tRNS_to_alpha(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB ||	color_type == PNG_COLOR_TYPE_GRAY)
		png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr); // Bitdepth 16 -> Bitdepth 8
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	//Prepare the return value
	SDL_Surface* RetVal = SDL_CreateRGBSurface(0,width,height,32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);
	guardRetVal.GuardSurface(RetVal);
	if(SDL_MUSTLOCK(RetVal))
		return 0;
	std::vector<png_byte*> RowPointers(height);
	for(unsigned int i = 0; i < height; i++)
		RowPointers[i] = (png_byte*) RetVal->pixels + RetVal->pitch * i;

	//Load the data

	png_read_image(png_ptr, &RowPointers.at(0));

	//Return
	guardRetVal.StopGuarding();
	return RetVal;
}

