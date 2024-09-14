FROM debian:bullseye-slim

RUN apt-get update && apt-get upgrade -y &&\
    apt-get install -y \
    build-essential

COPY . ./Webserv

WORKDIR /Webserv

RUN make && make clean &&\
    cp nginx++ ./www/other/binary &&\
    mv nginx++ /usr/local/bin &&\
    mv ./Configuration/servIO.conf /etc/

EXPOSE 8080 5500 3333

CMD ["nginx++"]