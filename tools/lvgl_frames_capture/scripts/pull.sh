#!/bin/bash
#
# Copy a recorded session off the EVB's eMMC and encode it.
#
#   pull.sh <board dir> [-session NNNN] [-src <dir>] [-o <dir>] [-rm] [-noencode]
#
# <board dir> is the demo's board folder that recorded, e.g.
# lvgl_ambiq_demo/lvgl_timeline/apollo510_evb. The session lands in
# <board dir>/lvcap/NNNN (or -o <dir>/NNNN) and is encoded there with
# encode.sh.
#
# The eMMC is read over USB: this flashes tools/LoadTexture, which exposes it
# as a USB drive on the EVB's USB-C port (not the J-Link one), so that cable
# must be connected. The board is left running LoadTexture; flash the demo
# again afterwards.
#
#   -session NNNN  which /lvcap session (default: the newest)
#   -src <dir>     the eMMC is already mounted at <dir> (skips LoadTexture);
#                  otherwise, under WSL, the drive is found through Windows
#   -rm            delete the session from the eMMC after copying it
#   -noencode      copy only
#
# Flashing uses $LVCAP_JLINK if set, as "$LVCAP_JLINK run <dir>" (a J-Link
# wrapper that flashes the .bin in <dir> and runs it), otherwise JLinkExe with
# LoadTexture's own flash.jlink.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PORT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
LOADTEXTURE="$PORT_ROOT/tools/LoadTexture"

usage() {
    sed -n '3,25p' "$0" | sed 's/^# \{0,1\}//'
}

BOARD=""
SESSION=""
SRC=""
OUT=""
RM=0
ENCODE=1
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)  usage; exit 0 ;;
        -session)   SESSION="$2"; shift 2 ;;
        -src)       SRC="$2"; shift 2 ;;
        -o)         OUT="$2"; shift 2 ;;
        -rm)        RM=1; shift ;;
        -noencode)  ENCODE=0; shift ;;
        -*)         echo "error: unknown option $1" >&2; usage >&2; exit 1 ;;
        *)          BOARD="$1"; shift ;;
    esac
done
[ -n "$BOARD" ] && [ -d "$BOARD" ] || { usage >&2; exit 1; }
OUT="${OUT:-$BOARD/lvcap}"
mkdir -p "$OUT"

# ---- the eMMC as a drive --------------------------------------------------------
ps() { powershell.exe -NoProfile -NonInteractive -Command "$1" | tr -d '\r'; }

if [ -z "$SRC" ]; then
    command -v powershell.exe >/dev/null \
        || { echo "error: not under WSL; mount the eMMC (LoadTexture, USB) and pass -src <dir>" >&2; exit 1; }

    echo "(PULL) flashing LoadTexture to expose the eMMC over USB"
    if [ -n "$LVCAP_JLINK" ]; then
        $LVCAP_JLINK run "$LOADTEXTURE/apollo510_evb" >/dev/null
    else
        (cd "$LOADTEXTURE" && JLinkExe -NoGui 1 -CommanderScript apollo510_evb/flash.jlink >/dev/null)
    fi

    # The removable FAT drive that has an lvcap folder.
    LETTER=""
    for _ in $(seq 1 30); do
        LETTER=$(ps "(Get-Volume | Where-Object { \$_.DriveType -eq 'Removable' -and \$_.DriveLetter -and (Test-Path (\$_.DriveLetter + ':\\lvcap')) } | Select-Object -First 1).DriveLetter")
        [ -n "$LETTER" ] && break
        sleep 1
    done
    [ -n "$LETTER" ] || { echo "error: no USB drive with an lvcap folder appeared; is the EVB's USB-C port connected?" >&2; exit 1; }
    WIN_ROOT="${LETTER}:\\lvcap"

    if [ -z "$SESSION" ]; then
        SESSION=$(ps "Get-ChildItem '$WIN_ROOT' -Directory | Where-Object { \$_.Name -match '^[0-9]+\$' } | Sort-Object { [int]\$_.Name } | Select-Object -Last 1 -ExpandProperty Name")
    fi
    [ -n "$SESSION" ] || { echo "error: no sessions in $WIN_ROOT" >&2; exit 1; }

    echo "(PULL) ${WIN_ROOT}\\$SESSION -> $OUT/$SESSION"
    rm -rf "${OUT:?}/$SESSION"
    ps "Copy-Item -Recurse -Force '$WIN_ROOT\\$SESSION' '$(wslpath -w "$OUT")'"
    [ $RM -eq 1 ] && ps "Remove-Item -Recurse -Force '$WIN_ROOT\\$SESSION'" && echo "(PULL) removed $SESSION from the eMMC"
else
    if [ -z "$SESSION" ]; then
        SESSION=$(ls "$SRC/lvcap" | grep -E '^[0-9]+$' | sort -n | tail -1)
    fi
    [ -n "$SESSION" ] || { echo "error: no sessions in $SRC/lvcap" >&2; exit 1; }
    echo "(PULL) $SRC/lvcap/$SESSION -> $OUT/$SESSION"
    rm -rf "${OUT:?}/$SESSION"
    cp -r "$SRC/lvcap/$SESSION" "$OUT/"
    [ $RM -eq 1 ] && rm -rf "$SRC/lvcap/$SESSION" && echo "(PULL) removed $SESSION from the eMMC"
fi

sed 's/^/        /' "$OUT/$SESSION/meta.txt" | tr -d '\r'

[ $ENCODE -eq 1 ] && bash "$SCRIPT_DIR/encode.sh" "$OUT/$SESSION"
exit 0
