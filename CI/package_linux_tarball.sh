#!/bin/bash
# Packs a TES3MP Linux build directory into a self-contained tree:
#   <out>/tes3mp, tes3mp-server, ...   real binaries (RPATH $ORIGIN/lib)
#   <out>/tes3mp.sh, tes3mp-server.sh   launchers (set LD_LIBRARY_PATH so OSG finds its plugins, cd to the tree)
#   <out>/lib/                          bundled shared libraries + osgPlugins-<ver>
#   <out>/server/                       CoreScripts (if given)
# MyGUI is linked statically (Ubuntu ships 3.4.2, OpenMW needs 3.4.3).
# Libraries that belong to the target system (glibc, GL/GPU drivers, X11/Wayland, audio servers) are not bundled.
#
# usage: package_linux_tarball.sh BUILD_DIR OUT_DIR [CORESCRIPTS_DIR]

set -euo pipefail

BUILD=$(realpath "$1")
OUT=$2
CORESCRIPTS=${3:-}

mkdir -p "$OUT/lib"
OUT=$(realpath "$OUT")

BINARIES=(tes3mp tes3mp-server openmw-iniimporter openmw-essimporter bsatool esmtool openmw-navmeshtool openmw-bulletobjecttool)
for b in "${BINARIES[@]}"; do
  if [ -x "$BUILD/$b" ]; then
    cp "$BUILD/$b" "$OUT/"
  fi
done
[ -x "$OUT/tes3mp" ] && [ -x "$OUT/tes3mp-server" ] || { echo "tes3mp or tes3mp-server missing in $BUILD" >&2; exit 1; }

for f in openmw.cfg defaults.bin gamecontrollerdb.txt tes3mp-client-default.cfg tes3mp-server-default.cfg; do
  cp "$BUILD/$f" "$OUT/"
done
cp -r "$BUILD/resources" "$OUT/"

# OSG plugins OpenMW uses (see USED_OSG_PLUGINS in CMakeLists.txt)
OSG_PLUGIN_DIR=$(dirname "$(find /usr/lib /usr/local/lib -name 'osgdb_png.so' -path '*osgPlugins-*' | head -n 1)")
OSG_PLUGIN_NAME=$(basename "$OSG_PLUGIN_DIR")
mkdir -p "$OUT/lib/$OSG_PLUGIN_NAME"
for p in bmp dae dds freetype jpeg osg png serializers_osg tga; do
  if [ -f "$OSG_PLUGIN_DIR/osgdb_$p.so" ]; then
    cp "$OSG_PLUGIN_DIR/osgdb_$p.so" "$OUT/lib/$OSG_PLUGIN_NAME/"
  else
    echo "warning: OSG plugin osgdb_$p.so not found" >&2
  fi
done

# Libraries expected on every desktop/server distro, or that must come from the target system (drivers)
EXCLUDE='^(ld-linux.*|linux-vdso.*|libc|libm|libdl|libpthread|librt|libresolv|libutil|libanl|libnsl|libstdc\+\+|libgcc_s|libz'\
'|libGL|libGLX|libGLdispatch|libEGL|libOpenGL|libGLESv[0-9]|libgbm|libdrm.*|libva.*|libvdpau|libnvidia.*|libcuda'\
'|libX11|libX11-xcb|libXext|libXau|libXdmcp|libXrender|libXrandr|libXi|libXfixes|libXcursor|libXinerama|libXxf86vm|libXss|libxcb.*|libxkbcommon.*'\
'|libwayland-.*|libdecor-.*|libasound|libpulse.*|libjack|libpipewire-.*|libdbus-1|libsystemd|libudev|libfontconfig|libexpat'\
'|libgmp|libgnutls|libp11-kit|libtasn1|libhogweed|libnettle|libidn2|libunistring|libffi|libselinux|libpcre2-8|libcap|libgcrypt|libgpg-error)\.so'

copy_deps() {
  ldd "$1" | awk '/=> \// { print $1 " " $3 }' | while read -r name path; do
    if [[ $name =~ $EXCLUDE ]]; then
      continue
    fi
    if [ ! -e "$OUT/lib/$name" ]; then
      cp -L "$path" "$OUT/lib/$name"
      echo "$OUT/lib/$name"
    fi
  done
}

# Collect libraries of the binaries and plugins, then of the newly copied libraries until nothing new appears
todo=$(find "$OUT" -maxdepth 1 -type f -executable ! -name '*.sh'; find "$OUT/lib/$OSG_PLUGIN_NAME" -name '*.so')
while [ -n "$todo" ]; do
  new=""
  for f in $todo; do
    new+=$(copy_deps "$f")$'\n'
  done
  todo=$(echo "$new" | sed '/^$/d' | sort -u)
done

for b in "${BINARIES[@]}"; do
  [ -f "$OUT/$b" ] && patchelf --set-rpath '$ORIGIN/lib' "$OUT/$b"
done
for l in "$OUT"/lib/*.so*; do
  patchelf --set-rpath '$ORIGIN' "$l"
done
for p in "$OUT/lib/$OSG_PLUGIN_NAME"/*.so; do
  patchelf --set-rpath '$ORIGIN/..' "$p"
done
strip --strip-debug "$OUT"/lib/*.so* "$OUT/lib/$OSG_PLUGIN_NAME"/*.so || true
# Debug info for the binaries goes to a separate archive (like the Windows pdb artifact)
if [ -n "${DEBUG_OUT:-}" ]; then
  mkdir -p "$DEBUG_OUT"
fi
for b in "${BINARIES[@]}"; do
  if [ -f "$OUT/$b" ]; then
    [ -n "${DEBUG_OUT:-}" ] && objcopy --only-keep-debug "$OUT/$b" "$DEBUG_OUT/$b.debug"
    strip --strip-debug "$OUT/$b"
  fi
done
du -sh "$OUT" "$OUT"/lib

for b in tes3mp tes3mp-server; do
  cat > "$OUT/$b.sh" <<EOF
#!/bin/sh
HERE=\$(dirname "\$(readlink -f "\$0")")
cd "\$HERE" || exit 1
export LD_LIBRARY_PATH="\$HERE/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}"
export OSG_LIBRARY_PATH="\$HERE/lib"
exec "\$HERE/$b" "\$@"
EOF
  chmod +x "$OUT/$b.sh"
done

if [ -n "$CORESCRIPTS" ]; then
  mkdir -p "$OUT/server"
  cp -r "$CORESCRIPTS"/. "$OUT/server/"
  rm -rf "$OUT/server/.git"
fi

cat > "$OUT/README-linux.txt" <<'EOF'
TES3MP Linux build (built on Ubuntu 24.04 with GCC 13)

Start the client with ./tes3mp.sh and the server with ./tes3mp-server.sh.
The launchers point the programs at the bundled libraries in lib/ and run them from this folder.

Client: set data= and content= in openmw.cfg (in this folder) or ~/.config/openmw/openmw.cfg, the same way as on
Windows, and the server address in tes3mp-client-default.cfg. Running openmw-iniimporter on Morrowind.ini can fill
in the content list.

Server: settings are in tes3mp-server-default.cfg. The server scripts (CoreScripts 0.8.1) are in server/; list the
client's plugins in server/data/requiredDataFiles.json.

Needs a 64-bit distro with glibc 2.39 or newer (Ubuntu 24.04, Debian 13, Fedora 40 or newer). GPU drivers, X11 or
Wayland and the sound system come from your system; the server needs none of those.
EOF
