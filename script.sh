#! /bin/sh
exec Xvfb $DISPLAY -screen 0 1920x1080x24 &
xce4-session &
sleep 4
firefox &
sleep 4
node .
echo "os Stated"
