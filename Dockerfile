FROM jlesage/firefox

RUN apk update
RUN apk add apache2

EXPOSE $PORT
