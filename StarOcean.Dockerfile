FROM debian:bullseye-slim

RUN apt-get update && apt-get install build-essential cmake git libboost-python-dev python3-pip -y

COPY . /

RUN pip install -r requirements.txt

RUN git clone https://github.com/RPGHacker/asar.git /opt/asar && \
    cd /opt/asar && \
    cmake src && make && \
    ln -s asar/bin/asar /usr/local/bin/asar && \
    ls -lh / && cd /romhacking && bash compile.sh


ENTRYPOINT ["python3","-m", "starocean.main"]
