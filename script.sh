#! /bin/sh
exec Xvfb $DISPLAY -screen 0 1024x768x24 &
fluxbox &
sleep 4
firefox &
sleep 4
node .
echo "os Stated"
