FROM ubuntu:22.04

EXPOSE 5000 5001

RUN apt update && apt install -y python3 python3-pip

RUN pip3 install BAC0 flask pytz

WORKDIR /app

COPY ./challenge/ /app

RUN chmod +x start.sh

CMD ["./start.sh"]
