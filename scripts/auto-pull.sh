#!/usr/bin/env bash
#
# auto-pull.sh
#     This script runs on zynq to:
#     - Pull changes from dev machine.
#     - Build if there are changes to source files, database, or Makefiles.
#     - Restart the detector (use `pkill -f GermaniumDetector`).
#
# Usage:  ./scripts/auto-pull.sh
#   or:   nohup ./scripts/auto-pull.sh &
#
# First-time setup (Zynq):
#   git clone liji@172.16.0.1:~/data/git/ZynqDetector /opt/ZynqDetector -b async-zmq

set -euo pipefail

### ── Configuration ───────────────────────────────────────────────────────────

POLL_INTERVAL=10
BRANCH="async-zmq"
BARE_REPO="$HOME/data/git/ZynqDetector.git"

ZYNQ_HOST="root@172.16.0.211"
ZYNQ_DIR="/opt/ZynqDetector"
ZYNQ_BUILD_CMD="cd ${ZYNQ_DIR}/src && make CXX=g++"

### ── End Configuration ───────────────────────────────────────────────────────

# Resolve this clone's root (the repo containing this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLONE_DIR="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)"

log() { printf '%s  %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"; }

cleanup() {
    trap - INT TERM
    echo ""
    log "Shutting down..."
    kill -- -$$ 2>/dev/null
}
trap cleanup INT TERM

zynq_pull_and_build() {
    log "--- Zynq update begin ---"
    log "SSH to $ZYNQ_HOST: checking current commit..."
    ssh "$ZYNQ_HOST" "cd $ZYNQ_DIR && git rev-parse --short HEAD" 2>/dev/null | while IFS= read -r line; do log "  zynq: current commit $line"; done

    log "SSH to $ZYNQ_HOST: git pull..."
    ssh "$ZYNQ_HOST" "cd $ZYNQ_DIR && git pull --ff-only origin $BRANCH" 2>&1 | while IFS= read -r line; do log "  zynq: $line"; done

    log "SSH to $ZYNQ_HOST: new commit..."
    zynq_commit=$(ssh "$ZYNQ_HOST" "cd $ZYNQ_DIR && git rev-parse --short HEAD" 2>/dev/null)
    log "  zynq: new commit $zynq_commit"

    log "SSH to $ZYNQ_HOST: build start..."
    build_log=$(ssh "$ZYNQ_HOST" "cd $ZYNQ_DIR/src && make CXX=g++" 2>&1)
    while IFS= read -r line; do log "  zynq: $line"; done <<< "$build_log"
    if echo "$build_log" | grep -q "error"; then
        log "Zynq build: FAILED"
    else
        log "Zynq build: SUCCESS"
        log "Restarting GermaniumDetector on Zynq..."
        ssh "$ZYNQ_HOST" "pkill -f GermaniumDetector"
    fi
    log "--- Zynq update end (commit $zynq_commit) ---"
