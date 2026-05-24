#!/bin/bash
set -euo pipefail

# Start the global stopwatch
SECONDS=0

# --- CONFIGURATION ---
CONTAINER_NAME="gtfs-db"
DB_USER="admin"
DB_NAME="gtfs-db"
COMPOSE_SERVICE="db"

# --- COLOR PALETTE ---
CYAN='\033[0;36m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
RED='\033[1;31m'
NC='\033[0m' # No Color

# --- UI HELPER FUNCTIONS ---
print_header() {
    echo -e "\n${CYAN}======================================================================${NC}"
    echo -e "${CYAN} 🚀 $1 ${NC}"
    echo -e "${CYAN}======================================================================${NC}"
}

print_info() { echo -e "${YELLOW}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }

# --- SQL EXECUTION WRAPPER ---
run_sql() {
    local file=$1
    (echo '\timing on'; cat "$file") | docker exec -i "$CONTAINER_NAME" psql -U "$DB_USER" -d "$DB_NAME" -v ON_ERROR_STOP=1 -a -P pager=off -P border=2 -P linestyle=unicode
}

# ==============================================================================
# PIPELINE START
# ==============================================================================
clear
print_header "STARTING GTFS INITIALIZATION PIPELINE"

print_info "Resetting container state..."
docker compose rm -s -v -f "$COMPOSE_SERVICE" > /dev/null 2>&1
docker compose up -d "$COMPOSE_SERVICE"

print_info "Waiting for PostgreSQL to accept connections..."
until docker exec "$CONTAINER_NAME" pg_isready -U "$DB_USER" -d "$DB_NAME" > /dev/null 2>&1; do
  echo -n "."
  sleep 2
done
echo "" # newline
print_success "PostgreSQL is awake and ready!"

# --- EXECUTION STEPS ---

print_header "STEP 1: Loading raw GTFS data"
run_sql "pipeline/01_raw_gtfs_init.sql"

print_header "STEP 2: Cleaning and filtering data"
run_sql "pipeline/02_gtfs_cleanup.sql"

# ==============================================================================
# PIPELINE END
# ==============================================================================
# Calculate total time
TOTAL_MIN=$(( SECONDS / 60 ))
TOTAL_SEC=$(( SECONDS % 60 ))

echo -e "\n${GREEN}======================================================================${NC}"
echo -e "${GREEN} 🎉 PIPELINE COMPLETE! The graph is ready for extraction.${NC}"
echo -e "${GREEN} ⏱️  Total Execution Time: ${TOTAL_MIN}m ${TOTAL_SEC}s${NC}"
echo -e "${GREEN}======================================================================${NC}\n"
