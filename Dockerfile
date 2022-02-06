FROM jlesage/firefox

RUN apk add apache2
RUN rc-service apache2 start

EXPOSE $PORT CMD [“apache2ctl”, “-D”, “FOREGROUND”]
