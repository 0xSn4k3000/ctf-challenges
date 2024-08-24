FROM php:8.2.12-cli

RUN apt-get update && apt-get install -y libpng-dev

RUN useradd -m www
RUN mkdir /app && chown www:www /app

COPY ./challenge/ /app

RUN chown -R www:www /app
WORKDIR /app

USER www

RUN random_value=$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c 32) && \
    hashed_value=$(echo -n $random_value | sha256sum | awk '{print $1}') && \
    mv flag.txt $hashed_value

RUN chmod +x ./start.sh

CMD ["./start.sh"]