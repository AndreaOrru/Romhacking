FROM archlinux:base-devel

RUN pacman -Syu --noconfirm
RUN pacman -S --noconfirm boost boost-libs cmake git python-pip

COPY . /app
WORKDIR /app

RUN git clone https://github.com/RPGHacker/asar /opt/asar && \
    cd /opt/asar &&                                          \
    cmake src && make &&                                     \
    ln -s asar/bin/asar /usr/local/bin/asar

RUN cd /app && pip install -r requirements.txt
RUN cd /app/romhacking && bash compile.sh

ENTRYPOINT ["python", "-m", "starocean.main"]
