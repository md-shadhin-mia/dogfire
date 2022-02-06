FROM jlesage/firefox
RUN apt-get update 
RUN apt-get install –y apache2 
RUN apt-get install –y apache2-utils 
RUN apt-get clean 
EXPOSE $PORT CMD [“apache2ctl”, “-D”, “FOREGROUND”]
