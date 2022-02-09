FROM node:17-alpine3.15

RUN apk add xvfb xfce4 firefox ffmpeg
RUN mkdir /home/app

ENV DISPLAY=:99

COPY ./package*.json /home/app/
COPY ./public/ /home/app/public
COPY ./app.js /home/app/
COPY ./script.sh /home/app/
WORKDIR /home/app
RUN npm i


CMD [ "sh", "script.sh" ]

EXPOSE 8000

