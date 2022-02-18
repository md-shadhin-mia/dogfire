#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct {
    int x, y;
	unsigned short red, green, blue;
} PixaColor;

PixaColor **pixelMap;


Bool isEqual(PixaColor *color1,PixaColor *color2){
    if (color1->red != color2->red)
    {
        return False;
    }
    if (color1->blue != color2->blue)
    {
        return False;
    }
    if (color1->green != color2->green)
    {
        return False;
    }
    return True;
}

void getPixelColor(XImage* img, int x, int y, PixaColor * color)
{
    unsigned long rgbcolor = XGetPixel(img, x, y);
    color->x = x;
    color->y = y;
    color->red = (rgbcolor & img->red_mask) >> 16 ;
    color->blue = rgbcolor & img->blue_mask;
    color->green = (rgbcolor & img->green_mask)>>8 ;
}


int main(int argc, char *argv[])
{
        Display *display;
        int screen;
        Window root;
        display = XOpenDisplay(0);
        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
        XEvent event;
        XWindowAttributes gwa;
        XGetWindowAttributes(display, root, &gwa);

        int width = gwa.width;
        int height = gwa.height;
        int x = 0;
        int y = 0;
        PixaColor curColor;

        pixelMap = (PixaColor**)malloc(sizeof(PixaColor*) * width);
        if (!pixelMap)
        {
            fprintf(stderr,"Allocation failed\n");
            return -1;
        }
        for (int i = 0; i < width; i++)
        {
            pixelMap[i] = (PixaColor*)malloc(sizeof(PixaColor) * height);
            if (!pixelMap[i])
            {
                fprintf(stderr,"Allocation failed\n");
                return -1;
            }
        }
        //initsialize image
        XImage *img = XGetImage(display,root,x,y,width-x,height-y,XAllPlanes(),ZPixmap);
        if (img != NULL){
            for (int x = 0; x < width; x++)
            {
                for (int y = 0; y < height; y++)
                {
                    getPixelColor(img, x, y, &curColor);
                    pixelMap[x][y] = curColor;
                }   
            }
            XDestroyImage(img);
        }
        int chageCound = 0;
        //test a color
        while (1)
        {
            img = XGetImage(display,root,0,0,width,height,XAllPlanes(),ZPixmap);
            if (img != NULL){
                for (int x = 0; x < width; x++)
                {
                    for (int y = 0; y < height; y++)
                    {
                        getPixelColor(img, x, y, &curColor);
                        if(!isEqual(&pixelMap[x][y], &curColor))
                        {
                            chageCound++;;
                            pixelMap[x][y] = curColor;
                        }
                    }   
                }
                XDestroyImage(img);
            }else{
                printf("image is null");
                return -1;
            }
            if(chageCound > 0)
            {
                printf("%d", chageCound);
                fflush(stdout);
                chageCound = 0;
            }
            usleep(1);
        }
        
        //clear up memory
        for (int i = 0; i < width; i++)
        {
            if(pixelMap[i])
                free(pixelMap[i]);
        }

        if (pixelMap)
            free(pixelMap);
        return 0;
}