ARG BASEIMAGE="alpine:3.23"
ARG GEOS_VERSION

FROM ${BASEIMAGE} AS builder

RUN apk add --no-cache \
  cmake \
  ninja \
  g++ \
  gcc

ADD . /source

ARG GEOS_VERSION
ENV GEOS_VERSION=${GEOS_VERSION}
RUN if [ -n "${GEOS_VERSION}" ]; then \
    rm -rf /source && mkdir /source && \
    wget https://github.com/libgeos/geos/archive/${GEOS_VERSION}.tar.gz \
      --output-document - | tar xz --directory=/source --strip-components=1; \
  fi

RUN cmake -S /source -B /build -G Ninja \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_INSTALL_LIBDIR=lib \
    -D BUILD_BENCHMARKS=OFF \
    -D BUILD_DOCUMENTATION=OFF \
    -D BUILD_TESTING=OFF \
  && cmake --build /build \
  && cmake --install /build --prefix /install --strip

FROM ${BASEIMAGE} AS runner

RUN apk add --no-cache libstdc++

COPY --from=builder /install/bin/geosop /usr/local/bin/geosop
COPY --from=builder /install/lib/libgeos.* /usr/local/lib/

ENTRYPOINT ["geosop"]
