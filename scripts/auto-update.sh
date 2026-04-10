#!/usr/bin/env bash
#
# monitor-github.sh — Poll GitHub, sync bare repo, and update Zynq via SSH.
#
# Runs on the Linux dev machine from the ZynqDetector clone.
# On new commits:
#   1. Pulls from GitHub into this clone
#   2. Pushes to the local bare repo
#   3. SSHs to the Zynq to pull and build
#
# Usage:  ./scripts/monitor-github.sh
#   or:   nohup ./scripts/monitor-github.sh &
#
# First-time setup (dev machine):
#   cd ~/data/git
#   git clone git@github.com:lijibnl/ZynqDetector.git -b async-zmq
#   git clone --bare git@github.com:lijibnl/ZynqDetector.git ZynqDetector.git
#
# First-time setup (Zynq):
#   git clone liji@172.16.0.1:~/data/git/ZynqDetector.git /opt/ZynqDetector -b async-zmq

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
    fi
    log "--- Zynq update end (commit $zynq_commit) ---"
}

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

        # 1. Pull into this clone
        git -C "$CLONE_DIR" pull --ff-only origin "$BRANCH" --quiet 2>&1 | \
            while IFS= read -r line; do log "  $line"; done
        log "Clone updated to $(git -C "$CLONE_DIR" rev-parse --short HEAD)"

        # 2. Push to bare repo
        if [[ -f "$BARE_REPO/HEAD" ]]; then
            git -C "$CLONE_DIR" push "$BARE_REPO" "$BRANCH:$BRANCH" --quiet 2>&1 | \
                while IFS= read -r line; do log "  bare: $line"; done
            log "Bare repo synced"
        else
            log "WARN: bare repo $BARE_REPO not found, skipping sync"
        fi

        # 3. SSH to Zynq: pull and build
        zynq_pull_and_build
    fi
}

log "Starting GitHub monitor (poll every ${POLL_INTERVAL}s, branch=$BRANCH)"
log "  Clone : $CLONE_DIR"
log "  Bare  : $BARE_REPO"
log "  Zynq  : $ZYNQ_HOST:$ZYNQ_DIR"

# Initial check on startup
poll_and_sync

while true; do
    sleep "$POLL_INTERVAL"
    poll_and_sync
done
