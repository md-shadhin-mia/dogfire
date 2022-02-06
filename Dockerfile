FROM jlesage/firefox

RUN apk --no-cache upgrade
RUN apk add --no-cache apache2

EXPOSE $PORT

CMD ["-D","FOREGROUND"]
