FROM jlesage/firefox
RUN yum -y install httpd
EXPOSE $PORT
