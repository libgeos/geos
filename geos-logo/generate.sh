
# web site title bar
convert geos-lg-white.png \
  -geometry 37x37 \
  -colorspace RGB \
  ../web-logo.png

# favicon
convert geos-lg-black.png \
  -geometry 32x32 \
  -colorspace RGB \
  -transparent white \
  ../favicon/favicon-32x32.png

# MSTile
for s in 70 150 310; do
  f=../favicon/mstile-${s}x${s}.png
  echo $f
  convert geos-lg-black.png \
    -geometry ${s}x${s} \
    -colorspace RGB \
    -transparent white \
    $f
done

# Android Favicon
for s in 70 150 310; do
  f=../favicon/android-chrome-${s}x${s}.png
  echo $f
  convert geos-lg-black.png \
    -geometry 192 \
    -colorspace RGB \
    -transparent white \
    $f
done

