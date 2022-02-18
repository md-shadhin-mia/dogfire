// #include <iostream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <png.h>
#include <stdio.h>

// using namespace std;
int colorRange = 255;
int getcolorrange(unsigned short c, int t)
{
   double tb = 255/t;
   double tc = (int)(c/tb+.5);
   int tr = tc*tb;
//    printf("%d , %d , %lf\n", tb, c, tc);
   return tr > 255? 255: tr;
}

void GetPix(XImage *image, int x, int y, XColor *color)
{
    unsigned long rgbcolor = XGetPixel(image, x, y);
    // printf("mask %x %x %x\n", image->red_mask >> 16, image->blue_mask, image->green_mask >> 8);
    color->red = (rgbcolor & image->red_mask) >> 16 ;
    color->blue = rgbcolor & image->blue_mask;
    color->green = (rgbcolor & image->green_mask)>>8 ;
    //color decrise
    color->red = getcolorrange(color->red, colorRange);
    color->blue = getcolorrange(color->blue, colorRange);
    color->green =  getcolorrange(color->green, colorRange);
    //back white
    // color->red = ((color->red+color->blue+color->green)/3) < 128 ? 0 : 255;
    // color->blue = ((color->red+color->blue+color->green)/3) < 128 ? 0 : 255;
    // color->green =  ((color->red+color->blue+color->green)/3) < 128 ? 0 : 255;
    //avarage
    // color->red = ((color->red+color->blue+color->green)/3);
    // color->blue = ((color->red+color->blue+color->green)/3);
    // color->green =  ((color->red+color->blue+color->green)/3);
}

void write_png_image(const char *filename, XImage *pImage)
{
    png_byte** row_pointers; // pointer to image bytes
    FILE* fp; // file for image
    XColor color;
    // GetPix(pImage, 25, 25, &color);
    int xc;
    do // one time do-while to properly free memory and close file after error
    {
        row_pointers = (png_byte**)malloc(sizeof(png_byte*) * pImage->height);
        if (!row_pointers)
        {
            printf("Allocation failed\n");
            break;
        }
        for (int i = 0; i < pImage->height; i++)
        {
            row_pointers[i] = (png_byte*)malloc(4*pImage->width);
            if (!row_pointers[i])
            {
                printf("Allocation failed\n");
                break;
            }
        }
        // fill image with color
        for (int y = 0; y < pImage->height; y++)
        {
            for (int x = 0; x < pImage->width; x+=1)
            {
                GetPix(pImage, x, y, &color);
                xc = x*4;
                row_pointers[y][xc] = color.red; //r
                row_pointers[y][xc + 1] = color.green; //g
                row_pointers[y][xc + 2] = color.blue; //b
                row_pointers[y][xc + 3] = 255; //a
            }
        }

        if(strcasecmp(filename, "-") == 0 || strcasecmp(filename, "pipe") == 0)
            fp = stdout; //create file for output
        else
            fp = fopen(filename, "wb");
        
        if (!fp)
        {
             fprintf(stderr,"Open file failed\n");
            break;
        }
        png_struct* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); //create structure for write
        if (!png)
        {
             fprintf(stderr,"Create write struct failed\n");
            break;
        }
        png_infop info = png_create_info_struct(png); // create info structure
        if (!info)
        {
             fprintf(stderr,"Create info struct failed\n");
            break;
        }
        if (setjmp(png_jmpbuf(png))) // this is some routine for errors?
        {
            fprintf(stderr,"setjmp failed\n");
        }
        png_init_io(png, fp); //initialize file output
        png_set_IHDR( //set image properties
            png, //pointer to png_struct
            info, //pointer to info_struct
            pImage->width, //image width
            pImage->height, //image height
            8, //color depth
            PNG_COLOR_TYPE_RGBA, //color type
            PNG_INTERLACE_NONE, //interlace type
            PNG_COMPRESSION_TYPE_DEFAULT, //compression type
            PNG_FILTER_TYPE_DEFAULT //filter type
            );
        png_write_info(png, info); //write png image information to file
        png_write_image(png, row_pointers); //the thing we gathered here for
        png_write_end(png, NULL);
    } while(0);
    //close file
    if (fp)
    {
        fclose(fp);
    }
    // free allocated memory
    for (int i = 0; i < pImage->height; i++)
    {
        if (row_pointers[i])
        {
            free(row_pointers[i]);
        }
    }
    if (row_pointers)
    {
        free(row_pointers);
    }
}

int main(int argc, char *argv[])
{
        Display *display;
        int screen;
        Window root;
        display = XOpenDisplay(0);
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
        XWindowAttributes gwa;
        XGetWindowAttributes(display, root, &gwa);

        int width = gwa.width;
        int height = gwa.height;
        int x = 0;
        int y = 0;
        char* filename;
        filename="";
        int opt;
        while ((opt = getopt(argc, argv , "x:y:w:h:o:r:") ) != -1)
        {
            switch (opt)
            {
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case 'o':
                filename = optarg;
                break;
            case 'r':
                colorRange = atoi(optarg);
                break;
            }
        }

        XImage *img = XGetImage(display,root,x,y,width,height,XAllPlanes(),ZPixmap);
        if (img != NULL)
        {
            if(strcmp(filename, "") != 0)
                write_png_image(filename, img);
        }
        return 0;
}