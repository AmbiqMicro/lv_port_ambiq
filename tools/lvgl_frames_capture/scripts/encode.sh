#!/bin/bash
#
# Encode a recorded session (frames.raw + meta.txt) to MP4 with ffmpeg.
#
#   encode.sh <session dir> [-o <out.mp4>] [-crf <n>]
#
# <session dir> is one /lvcap/NNNN folder copied off the eMMC, e.g. by
# pull.sh into <demo>/apollo510_evb/lvcap/NNNN. The output defaults to
# capture.mp4 inside it.
#
# frames.raw is headerless: meta.txt carries width, height, pix_fmt and fps,
# and frames.raw must hold exactly frames * width * height * 2 bytes.

set -e

usage() {
    sed -n '3,12p' "$0" | sed 's/^# \{0,1\}//'
}

SESSION=""
OUT=""
CRF=18
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help) usage; exit 0 ;;
        -o)   OUT="$2"; shift 2 ;;
        -crf) CRF="$2"; shift 2 ;;
        -*)   echo "error: unknown option $1" >&2; usage >&2; exit 1 ;;
        *)    SESSION="$1"; shift ;;
    esac
done
[ -n "$SESSION" ] || { usage >&2; exit 1; }
[ -f "$SESSION/meta.txt" ] && [ -f "$SESSION/frames.raw" ] \
    || { echo "error: $SESSION needs meta.txt and frames.raw" >&2; exit 1; }
command -v ffmpeg >/dev/null || { echo "error: ffmpeg not found" >&2; exit 1; }

meta() { sed -n "s/^$1=//p" "$SESSION/meta.txt" | tr -d '\r'; }
W=$(meta width); H=$(meta height); PIX=$(meta pix_fmt); FPS=$(meta fps)
FRAMES=$(meta frames); DROPPED=$(meta dropped)

SIZE=$(stat -c %s "$SESSION/frames.raw")
EXPECT=$((FRAMES * W * H * 2))
if [ "$SIZE" -ne "$EXPECT" ]; then
    echo "error: frames.raw is $SIZE bytes, meta.txt says $FRAMES frames of ${W}x${H} ($EXPECT)" >&2
    exit 1
fi
if [ "${DROPPED:-0}" -ne 0 ]; then
    echo "warning: $DROPPED frames were dropped while recording; the video is that much shorter than real time" >&2
fi

OUT="${OUT:-$SESSION/capture.mp4}"
ffmpeg -loglevel error -y -f rawvideo -pixel_format "$PIX" -video_size "${W}x${H}" -framerate "$FPS" \
    -i "$SESSION/frames.raw" -c:v libx264 -crf "$CRF" -pix_fmt yuv420p "$OUT"

echo "(ENCODE) $FRAMES frames ${W}x${H} @ $FPS fps ($(awk "BEGIN { printf \"%.1f\", $FRAMES / $FPS }") s) -> $OUT ($(du -h "$OUT" | cut -f1))"
