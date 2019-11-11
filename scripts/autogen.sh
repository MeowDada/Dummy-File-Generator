#!/bin/bash

GEN_DIR="dummy_collections"
GEN_ROOT="$HOME/Desktop/$GEN_DIR"
EXEC_PATH="../bin/dfgen"

FILENAME_PREFIX="dummy"
FILENAME_EXTENSION="tmp"

ratios=(0 10 20 30 40 50 60 70 80 90)
sizes=('10MB' '30MB' '50MB' '100MB')

# cleanup folders
rm -rf "$GEN_ROOT"

# create root folders
mkdir "$GEN_ROOT"

# create sub folders
for i in "${ratios[@]}"; do
    tmp="$GEN_ROOT/$i"
    mkdir $tmp
done

# creating files
for ratio in "${ratios[@]}"; do
    for size in "${sizes[@]}"; do
        $EXEC_PATH -f "$GEN_ROOT"/"$ratio"/"$FILENAME_PREFIX"_"$ratio"_"$size"."$FILENAME_EXTENSION" -s "$size" -r "$ratio"
    done
done



