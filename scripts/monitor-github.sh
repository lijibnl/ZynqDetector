#!/usr/bin/env bash
#
# monitor-github.sh — Poll GitHub and sync to local bare repo.
#
# Run from the ZynqDetector regular clone on the Linux machine.
# Pulls from GitHub into this clone, then pushes to the local bare repo
# so the Zynq can fetch from it.
#
# Usage:  ./scripts/monitor-github.sh
#   or:   nohup ./scripts/monitor-github.sh &
#
# First-time setup:
#   cd ~/data/git
#   git clone git@github.com:lijibnl/ZynqDetector.git -b async-zmq
#   git clone --bare git@github.com:lijibnl/ZynqDetector.git ZynqDetector.git
#   # (or: git init --bare ZynqDetector.git)

set -euo pipefail

### ── Configuration ───────────────────────────────────────────────────────────

POLL_INTERVAL=10
BRANCH="async-zmq"
BARE_REPO="$HOME/data/git/ZynqDetector.git"

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

poll_and_sync() {
    local local_sha remote_sha

    local_sha=$(git -C "$CLONE_DIR" rev-parse "$BRANCH" 2>/dev/null) || return
    git -C "$CLONE_DIR" fetch origin "$BRANCH" --quiet 2>/dev/null || {
        log "WARN: fetch from GitHub failed"
        return
    }
    remote_sha=$(git -C "$CLONE_DIR" rev-parse "origin/$BRANCH" 2>/dev/null) || return

    if [[ "$local_sha" != "$remote_sha" ]]; then
        log "New commits: ${local_sha:0:7} -> ${remote_sha:0:7}"

        # Pull into this clone
        git -C "$CLONE_DIR" pull --ff-only origin "$BRANCH" --quiet 2>&1 | \
            while IFS= read -r line; do log "  $line"; done
        log "Clone updated to $(git -C "$CLONE_DIR" rev-parse --short HEAD)"

        # Push to bare repo (so Zynq can fetch)
        if [[ -f "$BARE_REPO/HEAD" ]]; then
            git -C "$CLONE_DIR" push "$BARE_REPO" "$BRANCH:$BRANCH" --quiet 2>&1 | \
                while IFS= read -r line; do log "  bare: $line"; done
            log "Bare repo synced"
        else
            log "WARN: bare repo $BARE_REPO not found, skipping sync"
        fi
    fi
}

log "Starting GitHub monitor (poll every ${POLL_INTERVAL}s, branch=$BRANCH)"
log "  Clone   : $CLONE_DIR"
log "  Bare    : $BARE_REPO"

# Initial check on startup
poll_and_sync

while true; do
    sleep "$POLL_INTERVAL"
    poll_and_sync
done
