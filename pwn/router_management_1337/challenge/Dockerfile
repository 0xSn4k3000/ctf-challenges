FROM ubuntu:22.04
EXPOSE 1337
RUN groupadd ctf && useradd -m -g ctf ctf

COPY challenge/ /home/ctf/
WORKDIR /home/ctf
USER ctf
CMD ["./rm-1337"]

