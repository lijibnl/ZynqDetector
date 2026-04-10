#!/usr/bin/env bash
#
# monitor-and-build.sh — Poll remote bare repo, pull and build.
#
# Runs on the Zynq from the ZynqDetector clone.
# Polls liji@172.16.0.1:~/data/git/ZynqDetector.git for new commits,
# pulls into this clone, and rebuilds.
#
# Usage:  cd /opt/ZynqDetector && ./scripts/monitor-and-build.sh
#   or:   cd /opt/ZynqDetector && nohup ./scripts/monitor-and-build.sh &
#
# First-time setup on Zynq:
#   git clone liji@172.16.0.1:~/data/git/ZynqDetector.git /opt/ZynqDetector -b async-zmq

set -euo pipefail

### ── Configuration ───────────────────────────────────────────────────────────

POLL_INTERVAL=10
BRANCH="async-zmq"
REMOTE="liji@172.16.0.1:~/data/git/ZynqDetector.git"
BUILD_CMD=( make CXX=g++ )

### ── End Configuration ───────────────────────────────────────────────────────

# Resolve this clone's root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLONE_DIR="$(git -C "$SCRIPT_DIR" rev-parse --show-toplevel)"
SRC_DIR="$CLONE_DIR/src"

log() { printf '%s  %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"; }

cleanup() {
    trap - INT TERM
    echo ""
    log "Shutting down..."
    kill -- -$$ 2>/dev/null
}
trap cleanup INT TERM

# Ensure remote points to the bare repo on the Linux machine
setup_remote() {
    local current
    current=$(git -C "$CLONE_DIR" remote get-url origin 2>/dev/null) || true
    if [[ "$current" != "$REMOTE" ]]; then
        if [[ -n "$current" ]]; then
            git -C "$CLONE_DIR" remote set-url origin "$REMOTE"
        else
            git -C "$CLONE_DIR" remote add origin "$REMOTE"
        fi
        log "Remote set to $REMOTE"
    fi
}

pull_and_build() {
    local local_sha remote_sha

    local_sha=$(git -C "$CLONE_DIR" rev-parse "$BRANCH" 2>/dev/null) || return
    git -C "$CLONE_DIR" fetch origin "$BRANCH" --quiet 2>/dev/null || {
        log "WARN: fetch from $REMOTE failed"
        return
    }
    remote_sha=$(git -C "$CLONE_DIR" rev-parse "origin/$BRANCH" 2>/dev/null) || return

    if [[ "$local_sha" != "$remote_sha" ]]; then
        log "New commits: ${local_sha:0:7} -> ${remote_sha:0:7}"
        git -C "$CLONE_DIR" pull --ff-only origin "$BRANCH" --quiet 2>&1 | \
            while IFS= read -r line; do log "  $line"; done
        log "Updated to $(git -C "$CLONE_DIR" rev-parse --short HEAD)"

        log "Starting build..."
        if ( cd "$SRC_DIR" && "${BUILD_CMD[@]}" ); then
            log "Build succeeded."
        else
            log "Build FAILED (exit $?)."
        fi
    fi
}

if [[ ! -d "$CLONE_DIR/.git" ]]; then
    log "ERROR: $CLONE_DIR is not a git repo. Clone it first:"
    log "  git clone $REMOTE $CLONE_DIR -b $BRANCH"
    exit 1
fi

setup_remote

log "Starting ZynqDetector monitor (poll every ${POLL_INTERVAL}s, branch=$BRANCH)"
log "  Clone : $CLONE_DIR"
log "  Source: $SRC_DIR"
log "  Remote: $REMOTE"

# Initial check on startup
pull_and_build

while true; do
    sleep "$POLL_INTERVAL"
    pull_and_build
done
