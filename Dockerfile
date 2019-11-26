FROM alpine:3.10
RUN apk add make g++ autoconf build-base bash yasm \
  && rm -rf /var/cache/apk/* /tmp/*
