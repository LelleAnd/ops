#!/bin/bash

# exit on errors
set -e

# find out where this script is so we can invoke jar in ../jars
#SCRIPT_PATH=`dirname "$0"`; SCRIPT_PATH=`eval "cd \"$SCRIPT_PATH\" && pwd"`
SCRIPT_PATH=`readlink -f "$0"`; SCRIPT_PATH=`dirname "$SCRIPT_PATH"`; SCRIPT_PATH=`eval "cd \"$SCRIPT_PATH\" && pwd"`

#echo "invoking script at: $SCRIPT_PATH"
#CWD=$(pwd)
#echo "current working directory $CWD"

OPS_PATH=$SCRIPT_PATH/../../../Python
GEN_PATH=$SCRIPT_PATH/../../OPSIdls/TestAll/Generated/Python
#GEN_PATH2=$GEN_PATH/TestAll
#export PYTHONPATH=$OPS_PATH:$GEN_PATH:$GEN_PATH2:$PYTHONPATH
export PYTHONPATH=$OPS_PATH:$GEN_PATH:$PYTHONPATH

python $SCRIPT_PATH/VerifySerDes.py $@
