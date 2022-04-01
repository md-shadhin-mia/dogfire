#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct {
    int grid_size;
    int color_threshold;
    int min_changed_cells;
    int use_fast_mode;
    int adaptive_sleep;
    int base_sleep_us;
    int min_sleep_us;
} Config;

typedef struct {
    unsigned short red, green, blue;
    unsigned short avg;
} CellColor;

typedef struct {
    int x, y;
    CellColor color;
} GridCell;

volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

GridCell *gridMap;
CellColor *prevFrame;
Config config;


inline int colorDistance(CellColor *c1, CellColor *c2) {
    int dr = (int)c1->red - (int)c2->red;
    int dg = (int)c1->green - (int)c2->green;
    int db = (int)c1->blue - (int)c2->blue;
    return abs(dr) + abs(dg) + abs(db);
}

inline int isCellDifferent(CellColor *c1, CellColor *c2, int threshold) {
    return colorDistance(c1, c2) > threshold;
}

void getCellColor(XImage* img, int gridX, int gridY, int gridSize, int imgWidth, int imgHeight, CellColor *cell) {
    int startX = gridX * gridSize;
    int startY = gridY * gridSize;
    int endX = startX + gridSize;
    int endY = startY + gridSize;

    if (endX > imgWidth) endX = imgWidth;
    if (endY > imgHeight) endY = imgHeight;

    unsigned long totalRed = 0, totalGreen = 0, totalBlue = 0;
    int pixelCount = 0;

    if (img->bits_per_pixel == 32 || img->bits_per_pixel == 24) {
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                unsigned char *pixel = (unsigned char*)(img->data + y * img->bytes_per_line + x * (img->bits_per_pixel / 8));
                totalRed += pixel[0];
                totalGreen += pixel[1];
                totalBlue += pixel[2];
                pixelCount++;
            }
        }
    } else {
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                unsigned long rgb = XGetPixel(img, x, y);
                totalRed += (rgb & img->red_mask) >> 16;
                totalGreen += (rgb & img->green_mask) >> 8;
                totalBlue += rgb & img->blue_mask;
                pixelCount++;
            }
        }
    }

    cell->red = totalRed / pixelCount;
    cell->green = totalGreen / pixelCount;
    cell->blue = totalBlue / pixelCount;
    cell->avg = (cell->red + cell->green + cell->blue) / 3;
}


void printUsage(const char *progname) {
    fprintf(stderr, "Usage: %s [options]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -g SIZE     Grid size for downsampling (default: 4)\n");
    fprintf(stderr, "  -t THRESH   Color difference threshold (default: 10)\n");
    fprintf(stderr, "  -m MIN      Min changed cells to trigger (default: 1)\n");
    fprintf(stderr, "  -s US       Base sleep time in microseconds (default: 40000)\n");
    fprintf(stderr, "  -n          Disable adaptive sleep\n");
    fprintf(stderr, "  -h          Show this help\n");
}

void parseArgs(int argc, char *argv[]) {
    config.grid_size = 4;
    config.color_threshold = 10;
    config.min_changed_cells = 1;
    config.use_fast_mode = 1;
    config.adaptive_sleep = 1;
    config.base_sleep_us = 40000;
    config.min_sleep_us = 10000;

    int opt;
    while ((opt = getopt(argc, argv, "g:t:m:s:nh")) != -1) {
        switch (opt) {
            case 'g': config.grid_size = atoi(optarg); break;
            case 't': config.color_threshold = atoi(optarg); break;
            case 'm': config.min_changed_cells = atoi(optarg); break;
            case 's': config.base_sleep_us = atoi(optarg); break;
            case 'n': config.adaptive_sleep = 0; break;
            case 'h': printUsage(argv[0]); exit(0);
            default: printUsage(argv[0]); exit(1);
        }
    }

    if (config.grid_size < 1) config.grid_size = 1;
    if (config.color_threshold < 0) config.color_threshold = 0;
    if (config.min_changed_cells < 1) config.min_changed_cells = 1;
}

int main(int argc, char *argv[])
{
        Display *display = NULL;
        XImage *img = NULL;
        int screen;
        Window root;
        int width, height;
        int exit_code = 0;

        parseArgs(argc, argv);

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
#ifndef _WIN32
        signal(SIGHUP, signal_handler);
#endif

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

        if (width <= 0 || height <= 0) {
            fprintf(stderr, "Error: Invalid screen dimensions (%dx%d)\n", width, height);
            XCloseDisplay(display);
            return -1;
        }

        int gridWidth = (width + config.grid_size - 1) / config.grid_size;
        int gridHeight = (height + config.grid_size - 1) / config.grid_size;
        size_t totalCells = (size_t)gridWidth * gridHeight;

        fprintf(stderr, "Screen: %dx%d, Grid: %dx%d (%zu cells), Threshold: %d\n",
                width, height, gridWidth, gridHeight, totalCells, config.color_threshold);

        gridMap = (GridCell*)calloc(totalCells, sizeof(GridCell));
        prevFrame = (CellColor*)calloc(totalCells, sizeof(CellColor));
        if (gridMap == NULL || prevFrame == NULL) {
            fprintf(stderr, "Error: Memory allocation failed for grid\n");
            if (gridMap) free(gridMap);
            if (prevFrame) free(prevFrame);
            XCloseDisplay(display);
            return -1;
        }

        for (int gy = 0; gy < gridHeight; gy++) {
            for (int gx = 0; gx < gridWidth; gx++) {
                gridMap[gy * gridWidth + gx].x = gx;
                gridMap[gy * gridWidth + gx].y = gy;
            }
        }

        img = XGetImage(display, root, 0, 0, width, height, XAllPlanes(), ZPixmap);
        if (img == NULL) {
            fprintf(stderr, "Error: Cannot capture initial screen image\n");
            free(gridMap);
            free(prevFrame);
            XCloseDisplay(display);
            return -1;
        }

        for (int gy = 0; gy < gridHeight; gy++) {
            for (int gx = 0; gx < gridWidth; gx++) {
                getCellColor(img, gx, gy, config.grid_size, width, height, &gridMap[gy * gridWidth + gx].color);
                prevFrame[gy * gridWidth + gx] = gridMap[gy * gridWidth + gx].color;
            }
        }
        XDestroyImage(img);
        img = NULL;

        int changeCount = 0;
        int maxCX = 0, minCX = gridWidth;
        int maxCY = 0, minCY = gridHeight;
        int frameCount = 0;

        while (running) {
            img = XGetImage(display, root, 0, 0, width, height, XAllPlanes(), ZPixmap);
            if (img == NULL) {
                fprintf(stderr, "Warning: Failed to capture screen, retrying...\n");
                usleep(100000);
                continue;
            }

            changeCount = 0;
            maxCX = 0; minCX = gridWidth;
            maxCY = 0; minCY = gridHeight;

            for (int gy = 0; gy < gridHeight; gy++) {
                for (int gx = 0; gx < gridWidth; gx++) {
                    getCellColor(img, gx, gy, config.grid_size, width, height, &gridMap[gy * gridWidth + gx].color);

                    if (isCellDifferent(&prevFrame[gy * gridWidth + gx], &gridMap[gy * gridWidth + gx].color, config.color_threshold)) {
                        changeCount++;
                        if (gx > maxCX) maxCX = gx;
                        if (gy > maxCY) maxCY = gy;
                        if (gx < minCX) minCX = gx;
                        if (gy < minCY) minCY = gy;
                        prevFrame[gy * gridWidth + gx] = gridMap[gy * gridWidth + gx].color;
                    }
                }
            }
            XDestroyImage(img);
            img = NULL;

            if (changeCount >= config.min_changed_cells) {
                int pixelMinX = minCX * config.grid_size;
                int pixelMinY = minCY * config.grid_size;
                int pixelWidth = (maxCX - minCX + 1) * config.grid_size;
                int pixelHeight = (maxCY - minCY + 1) * config.grid_size;

                printf("{\"pixel\":%d,\"point\":{\"x\":%d, \"y\":%d},\"to\":{\"w\":%d, \"h\":%d}}\n",
                       changeCount, pixelMinX, pixelMinY, pixelWidth, pixelHeight);
                fflush(stdout);
            }

            int sleepTime = config.base_sleep_us;
            if (config.adaptive_sleep && changeCount > 0) {
                float activityRatio = (float)changeCount / totalCells;
                sleepTime = config.base_sleep_us - (activityRatio * (config.base_sleep_us - config.min_sleep_us));
                if (sleepTime < config.min_sleep_us) sleepTime = config.min_sleep_us;
            }

            usleep(sleepTime);
            frameCount++;
        }

        fprintf(stderr, "\nProcessed %d frames, shutting down...\n", frameCount);

cleanup:
        if (gridMap) {
            free(gridMap);
            gridMap = NULL;
        }
        if (prevFrame) {
            free(prevFrame);
            prevFrame = NULL;
        }

        if (display) {
            XCloseDisplay(display);
        }

        return exit_code;
}
