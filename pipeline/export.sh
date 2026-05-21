#!/bin/bash
echo "Extracting Metro Graph from PostgreSQL..."

# Define your docker and DB variables
CONTAINER_NAME="gtfs-db"
DB_USER="admin"
DB_NAME="gtfs-db"

# Create the node header file
docker exec -i $CONTAINER_NAME psql -U $DB_USER -d $DB_NAME -t -A -q -f /pipeline/03_nodes_export.sql | tr -d '\r' > ./engine/include/node.hpp

# Create the edge header file
docker exec -i $CONTAINER_NAME psql -U $DB_USER -d $DB_NAME -t -A -q -f /pipeline/04_edges_export.sql | tr -d '\r' > ./engine/include/edge.hpp
