#! /bin/sh
exec Xvfb $DISPLAY -screen 0 1920x1080x24 &
lxde-session &
sleep 4
firefox &
sleep 4
node .
echo "os Stated"
