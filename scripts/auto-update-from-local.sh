#!/usr/bin/env bash
#
# auto-update-from-local.sh
#     This script runs on dev machine to:
#     - Continuously monitor changes (inotifywait) to local files (source, db, Makefiles)
#     - On changes, rsync to zynq, then ssh to:
#       - Build if there are changes to source files, database, or Makefiles.
#       - Kill the process (use `pkill -f GermaniumDetector`) only if build succeeds.
#
# The directory on zynq is /opt/ZynqDetector, which is a clone of this repo.
#
# Usage:  ./scripts/auto-update-from-local.sh
#
ZYNQ_HOST="root@172.16.0.211"
ZYNQ_DIR="/opt/ZynqDetector"
ZYNQ_BUILD_CMD="cd ${ZYNQ_DIR}/src && make CXX=g++"

# Directories and file extensions to watch
WATCH_DIRS=("./src")
WATCH_EXTS="c|h|cpp|hpp|tpp|Makefile"
DEBOUNCE=0.8

# Rsync exclude patterns (from .gitignore)
RSYNC_EXCLUDES=(
  --exclude '/build' --exclude '/export' --exclude '/out'
  --exclude '*.o' --exclude '*.d'
  --exclude '/logs' --exclude '*.log'
  --exclude '*.lock'
  --exclude '/src/bin' --exclude '/src/build' --exclude '/src/GermaniumDetector'
  --exclude '*.bin' --exclude '*.pdi' --exclude 'vitisWorkspace.json'
  --exclude '_ide/logs' --exclude '_ide/.wsdata'
  --exclude '/src/.vscode' --exclude '.vscode'
)

log() { printf '%s  %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"; }

# Build the inotifywait command
INOTIFY_ARGS=( -m -r -e close_write,moved_to,delete --format '%w%f' )
for dir in "${WATCH_DIRS[@]}"; do
  INOTIFY_ARGS+=( "$dir" )
done

# Debounce function
debounce() {
  local last_event=0
  while read -r file; do
    now=$(date +%s.%N)
    last_event=$now
    sleep $DEBOUNCE
    now2=$(date +%s.%N)
    # If no new event during debounce, proceed
    if [[ "$last_event" == "$now" ]]; then
      echo "$file"
    fi
  done
}

log "Monitoring for changes..."

inotifywait "${INOTIFY_ARGS[@]}" | \
  grep --line-buffered -E "\\.($WATCH_EXTS)$|Makefile$" | \
  debounce | \
  while read -r changed_file; do
    log "Change detected: $changed_file"
    log "Syncing to Zynq..."
    log "Running: rsync -avz --delete ${RSYNC_EXCLUDES[*]} ./ $ZYNQ_HOST:$ZYNQ_DIR/"
    rsync -avz --delete "${RSYNC_EXCLUDES[@]}" ./ "$ZYNQ_HOST:$ZYNQ_DIR/"
    log "Triggering build on Zynq..."
    ssh "$ZYNQ_HOST" "$ZYNQ_BUILD_CMD" > build.log 2>&1
    if ssh "$ZYNQ_HOST" "grep -q 'error' $ZYNQ_DIR/src/build.log"; then
      log "Build failed on Zynq. Not restarting detector."
    else
      log "Build succeeded. Restarting GermaniumDetector on Zynq."
      ssh "$ZYNQ_HOST" "pkill -f GermaniumDetector || true"
    fi
  done

