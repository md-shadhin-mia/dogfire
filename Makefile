all:
	gcc -o takepng takepng.c -lpng -lX11
	gcc -o chage-monitor chageMonitor.c -lX11
