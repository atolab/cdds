##!/bin/sh


OUTPUT_DIR=
SCRIPT_DIR=
EXAMPLES_DIR=
EXPLICIT_YES=false


show_help() {
cat << EOF
Usage: ${0##*/} [-h] [-y] [-d OUTDIR]

The VortexDDS examples are probably installed in a read-only location.
By executing this script, the examples can be (re)installed to a writable
location. That could be helpful when trying to experiment with the examples.

    -d|--dir OUTDIR  Install the examples in OUTDIR.
                     This directory should not be a sub-directory of the
                     examples location.
                     If not set, an output dir will be asked for. When asking
                     for an output dir, the current directory is used as
                     suggestion.
    -h|--help        This text.
    -y|--yes         Use 'yes' for every question.
EOF
}


#
# Parse command line arguments.
#
if [ -z "$1" ]; then
    show_help
    printf '\n'
else
    while :; do
        case $1 in
            -h|-\?|--help)
                show_help
                exit
                ;;
            -d|--dir)
                if [ "$2" ]; then
                    OUTPUT_DIR=$2
                    shift
                else
                    show_help
                    printf '\nERROR: "-d|--dir" requires a non-empty option argument.\n' "$1" >&2
                    exit 1
                fi
                ;;
            -y|--yes)
                EXPLICIT_YES=true
                ;;
            -?*)
                printf 'WARN: Unknown option (ignored): %s\n' "$1" >&2
                ;;
            *)
                break
        esac
        shift
    done
fi



#
# Get the location of the script.
#
SCRIPT=`readlink -f "$0"`
SCRIPT_DIR=`dirname "$SCRIPT"`


#
# The examples are in the same location as the script.
#
EXAMPLES_DIR="$SCRIPT_DIR"


#
# Only get the output dir ourselves when it wasn't already set by the
# command line arguments.
#
if [ -z "$OUTPUT_DIR" ]; then
    # Assume the examples should be installed in the current directory.
    OUTPUT_DIR=`pwd`

    # When explicit 'yes' is provided as a command line argument, then
    # don't ask if the assumption is correct.
    if [ $EXPLICIT_YES = false ]; then

        # Keep pestering the user until we have a proper answer.
        while true; do
            YNC=
            if [ "$OUTPUT_DIR" = "$SCRIPT_DIR" ]; then
                YNC="N"
            else
                read -p "Do you wish to install the VortexDDS examples in \"$OUTPUT_DIR\"? [Yes|No|Cancel] " YNC
            fi
            case $YNC in
                [Yy]* ) break;;
                [Nn]* ) read -p "New examples install directory> " OUTPUT_DIR; break;;
                [Cc]* ) exit;;
                * ) echo "Please answer yes, no or cancel.";;
            esac
        done
    elif [ "$OUTPUT_DIR" = "$SCRIPT_DIR" ]; then
        show_help
        printf '\nERROR: Script executed in example root without destination.\n'
        exit 1
    fi
fi


#
# Check if the output dir is valid.
#
if [ ! -d "$OUTPUT_DIR" ]; then
    # Only ask for permission if an explicit yes wasn't part of
    # the command line arguments.
    if [ $EXPLICIT_YES = false ]; then
        while true; do
            read -p "Do you wish to create directory \"$OUTPUT_DIR\"? [Yes|No] " YN
            case $YN in
                [Yy]* ) break;;
                [Nn]* ) exit;;
                * ) echo "Please answer yes or no.";;
            esac
        done
    fi
    mkdir -p "$OUTPUT_DIR"
    if [ $? -ne 0 ]; then
        printf 'ERROR: Could not create directory "%s"\n' "$OUTPUT_DIR"
        exit 1
    fi
fi
# If the directory still doesn't exist, exit.
if [ ! -d "$OUTPUT_DIR" ]; then
    show_help
    printf '\nERROR: Directory "%s" does not exist.\n' "$OUTPUT_DIR" >&2
    exit 1
fi
# If the directory isn't writable, exit.
if [ ! -w "$OUTPUT_DIR" ]; then
    show_help
    printf '\nERROR: Directory "%s" does not have write permission.\n' "$OUTPUT_DIR" >&2
    exit 1
fi


#
# Copy the examples.
#
cp -Rf "$EXAMPLES_DIR" "$OUTPUT_DIR"
if [ $? -ne 0 ]; then
    printf 'ERROR: Could not install examples\n'
    exit 1
else
    printf 'Installed VortexDDS examples into "%s"\n' "$OUTPUT_DIR"
fi

