FROM node

RUN apt update
RUN apt install -y xvfb fluxbox firefox-esr xdotool
RUN mkdir /home/app
COPY ./takepng.c /home/app/
COPY ./chageMonitor.c /home/app/
COPY ./Makefile /home/app/
WORKDIR /home/app
RUN make
ENV DISPLAY=:99
COPY ./package*.json /home/app/
COPY ./public/ /home/app/public
COPY ./app.js /home/app/
COPY ./script.sh /home/app/
WORKDIR /home/app
RUN npm i
CMD [ "sh", "script.sh" ]
# RUN apk add xvfb fluxbox firefox ffmpeg
# RUN mkdir /home/app

# ENV DISPLAY=:99

# COPY ./package*.json /home/app/
# COPY ./public/ /home/app/public
# COPY ./app.js /home/app/
# COPY ./script.sh /home/app/
# WORKDIR /home/app
# RUN npm i


# CMD [ "sh", "script.sh" ]

EXPOSE 8000

