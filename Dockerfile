FROM debian:buster-slim
RUN apt update \
  && apt install -y --no-install-recommends --no-install-suggests \
        cmake make g++ yasm wget \
        build-essential libz-dev \
        liblog4cxx-dev libghc-curl-dev libglib2.0-dev \
        libavcodec-dev libavutil-dev libavformat-dev libavresample-dev \
  && wget --no-check-certificate https://pkg-config.freedesktop.org/releases/pkg-config-0.29.2.tar.gz \
  && tar -zxf pkg-config-0.29.2.tar.gz \
  && cd pkg-config-0.29.2 \
  && ./configure  --with-internal-glib \
  && make && make install \
  && cp pkg-config /usr/local/bin \
  && rm -rf ../pkg-config-0.29.2 \
  && ln -s  /usr/lib/x86_64-linux-gnu/libgnutls.so.30.14.10 /usr/lib/x86_64-linux-gnu/libgnutls.so \
  && wget -c -t 0 -nv -O /opt/boost.tar.gz --no-check-certificate http://192.168.10.200/boost_1_70_0.tar.gz \
  && cd /opt \
  && tar -zxf boost.tar.gz \
  && cd boost_1_70_0 \
  && ./bootstrap.sh \
  && ./bjam \
  && ./bjam install \
  && echo '/usr/local/lib' >> /etc/ld.so.conf \
  && ldconfig \
  && rm -rf /opt/* \
  && rm -rf /var/lib/apt/lists/*
  