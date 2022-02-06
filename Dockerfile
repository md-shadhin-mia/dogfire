FROM alpine

RUN apk --no-cache upgrade
RUN apk add --no-cache apache2
RUN mkdir /run/apache2

EXPOSE $PORT

CMD ["-D","FOREGROUND"]
ENTRYPOINT ["/usr/sbin/httpd"]
