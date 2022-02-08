FROM node:17-alpine3.15

RUN apk add xvfb xfce4 firefox ffmpeg
RUN mkdir /home/app

ENV DISPLAY=:99
WORKDIR /home/app
COPY ./package*.json ./
RUN npm i
COPY ./public ./public
COPY ./app.js ./
ADD ./script.sh ./script.sh
CMD [ "sh", "script.sh" ]

EXPOSE 8000

