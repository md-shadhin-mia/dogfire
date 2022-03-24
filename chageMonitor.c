#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct {
    int x, y;
	unsigned short red, green, blue;
} PixaColor;

// Global flag for graceful shutdown
volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

PixaColor *pixelMap; // Contiguous 1D array for better memory locality


Bool isEqual(PixaColor *color1,PixaColor *color2){
    if (color1 == NULL || color2 == NULL) {
        return False;
    }
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
        Display *display = NULL;
        XImage *img = NULL;
        int screen;
        Window root;
        int width, height;
        int exit_code = 0;

        // Set up signal handlers for graceful shutdown
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
#ifndef _WIN32
        signal(SIGHUP, signal_handler);
#endif

        // Open display with error handling
        display = XOpenDisplay(NULL);
        if (display == NULL) {
            fprintf(stderr, "Error: Cannot open display\n");
            return -1;
        }

        screen = DefaultScreen(display);
        root = RootWindow(display, screen);
        
        XWindowAttributes gwa;
        if (XGetWindowAttributes(display, root, &gwa) == 0) {
            fprintf(stderr, "Error: Cannot get window attributes\n");
            XCloseDisplay(display);
            return -1;
        }

        width = gwa.width;
        height = gwa.height;
        
        // Validate dimensions
        if (width <= 0 || height <= 0) {
            fprintf(stderr, "Error: Invalid screen dimensions (%dx%d)\n", width, height);
            XCloseDisplay(display);
            return -1;
        }

        // Allocate contiguous 1D array (better memory locality and less fragmentation)
        size_t total_pixels = (size_t)width * height;
        pixelMap = (PixaColor*)malloc(sizeof(PixaColor) * total_pixels);
        if (pixelMap == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for %zu pixels\n", total_pixels);
            XCloseDisplay(display);
            return -1;
        }

        // Helper macro for 2D indexing
        #define PIXEL(x, y) pixelMap[(y) * width + (x)]

        // Capture initial screen state
        img = XGetImage(display, root, 0, 0, width, height, XAllPlanes(), ZPixmap);
        if (img == NULL) {
            fprintf(stderr, "Error: Cannot capture initial screen image\n");
            free(pixelMap);
            XCloseDisplay(display);
            return -1;
        }

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                getPixelColor(img, x, y, &PIXEL(x, y));
                PIXEL(x, y).x = x;
                PIXEL(x, y).y = y;
            }
        }
        XDestroyImage(img);
        img = NULL;
        int chageCound = 0;
        int maxCX = 0;
        int minCX = width;
        int maxCY = 0;
        int minCY = height;
        PixaColor curColor;

        // Main monitoring loop
        while (running) {
            img = XGetImage(display, root, 0, 0, width, height, XAllPlanes(), ZPixmap);
            if (img == NULL) {
                fprintf(stderr, "Warning: Failed to capture screen, retrying...\n");
                usleep(100000); // Wait longer on error
                continue;
            }

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    getPixelColor(img, x, y, &curColor);
                    if (!isEqual(&PIXEL(x, y), &curColor)) {
                        chageCound++;
                        if (maxCX < x) maxCX = x;
                        if (maxCY < y) maxCY = y;
                        if (minCX > x) minCX = x;
                        if (minCY > y) minCY = y;
                        PIXEL(x, y) = curColor;
                    }
                }
            }
            XDestroyImage(img);
            img = NULL;

            if (chageCound > 0) {
                printf("{\"pixel\":%d,\"point\":{\"x\":%d, \"y\":%d},\"to\":{\"w\":%d, \"h\":%d}}\n",
                            chageCound, minCX, minCY, maxCX - minCX, maxCY - minCY);
                fflush(stdout);
                chageCound = 0;
                maxCX = 0;
                minCX = width;
                maxCY = 0;
                minCY = height;
            }
            usleep(40000);
        }

        printf("\nShutting down gracefully...\n");
        fflush(stdout);

cleanup:
        // Free memory
        if (pixelMap) {
            free(pixelMap);
            pixelMap = NULL;
        }
        
        // Close display
        if (display) {
            XCloseDisplay(display);
        }

        return exit_code;
}
