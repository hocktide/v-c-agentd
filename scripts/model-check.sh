#!/usr/bin/env bash

#Sanity check: is the first argument a valid directory?
MODEL_DIR=${MESON_SOURCE_ROOT}/$1

if [ ! -d "$MESON_SOURCE_ROOT" ]; then
    echo "First argument must be the models directory."
    exit 1
fi

#grab the model makefiles
MODEL_MAKEFILES=`cd $MODEL_DIR && find . -name "*.mk" -print`

for n in $MODEL_MAKEFILES; do
    echo "Running $n."
    if (cd $MODEL_DIR && make -f $n); then
        echo "$n succeeded."
    else
        echo "$n failed."
        exit 1
    fi
done
