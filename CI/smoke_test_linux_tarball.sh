#!/bin/bash
# Runs inside a clean ubuntu:24.04 container: unpacks the Linux tarball, runs the server on the bare system, then
# installs a desktop's base libraries and checks the client starts.
# usage: smoke_test_linux_tarball.sh TARBALL

set -euo pipefail

mkdir /t
tar -C /t -xzf "$1"
cd /t/*/

missing() {
  LD_LIBRARY_PATH=lib ldd "$@" | grep "not found" | sort -u || true
}

echo "== server: missing libraries =="
m=$(missing tes3mp-server)
echo "$m"
[ -z "$m" ]

sed -i "s/^enabled = true/enabled = false/" tes3mp-server-default.cfg
set +e
timeout 15 ./tes3mp-server.sh > server.log 2>&1
code=$?
set -e
cat server.log
if [ $code -ne 124 ]; then
  echo "server exited with $code before the timeout"
  exit 1
fi

export DEBIAN_FRONTEND=noninteractive
apt-get -qq update
apt-get -qq install -y --no-install-recommends libgl1 libegl1 libx11-6 libx11-xcb1 libxext6 libxrender1 libxrandr2 \
  libxi6 libxfixes3 libxcursor1 libxinerama1 libxss1 libxxf86vm1 libxkbcommon0 libwayland-client0 libwayland-cursor0 \
  libwayland-egl1 libdecor-0-0 libasound2t64 libpulse0 libfontconfig1 libdrm2 libgbm1 libva2 libva-drm2 libva-x11-2 \
  libvdpau1 > /dev/null

echo "== client: missing libraries =="
m=$(missing tes3mp lib/*.so* lib/osgPlugins-*/*.so)
echo "$m"
[ -z "$m" ]

./tes3mp.sh --version
