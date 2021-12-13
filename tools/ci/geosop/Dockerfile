FROM docker.osgeo.org/geos/build-test:alpine AS base
ARG VERSION=47f145e

WORKDIR /source
ADD . /source

#RUN wget --quiet https://github.com/libgeos/geos/archive/${VERSION}.tar.gz --output-document - \
#| tar xz --directory=. --strip-components=1

WORKDIR /build
RUN cmake -D CMAKE_BUILD_TYPE=Release -D BUILD_BENCHMARKS=OFF -D BUILD_TESTING=OFF /source \
  && make install \
  && cp bin/geosop /usr/local/bin/geosop

WORKDIR /install/bin
RUN cp --no-dereference /usr/local/bin/geos* . \
  && for i in ./*; do strip -s $i 2>/dev/null || /bin/true; done

WORKDIR /install/lib
RUN cp --no-dereference /usr/local/lib*/libgeos*.so.* . \
  && for i in ./*; do strip -s $i 2>/dev/null || /bin/true; done

FROM alpine
RUN apk add --no-cache libstdc++
COPY --from=base /install/bin/geos* /usr/local/bin/
COPY --from=base /install/lib/libgeos* /usr/local/lib/

ENTRYPOINT ["geosop"]
