#!/bin/bash

# Initialize variables
input_file="$1"
output_file="$2"
package_name="forest"

# Function to convert date format
convert_date() {
    date -d "$1" "+%a, %d %b %Y %H:%M:%S %z"
}

do_by_line(){
  echo # newline
  echo " -- Nicholas Yoder <nicholasyoder@proton.me>  $1"
  echo # newline
  echo # newline
}

if [[ -z "$input_file" || -z "$output_file" ]]; then
  echo "Usage: convert_changelog.sh ./changelog.md ../debian/changelog"
  exit 0
fi

# Process the file
{
    while IFS= read -r line; do
        if [[ $line =~ ^"* Release "([0-9.]+)" - "([0-9]{4}-[0-9]{2}-[0-9]{2})$ ]]; then
            release_type="release"
            version="${BASH_REMATCH[1]}"
            release_date=$(convert_date "${BASH_REMATCH[2]}")
            echo "$package_name ($version) unstable; urgency=medium"
            echo # newline
        elif [[ $line =~ ^"  - "(.+)$ ]]; then
            echo "  * ${BASH_REMATCH[1]}"
        elif [[ $line == "* Unreleased" ]]; then
            echo "$package_name ($(date +%Y.%m.%d)) UNRELEASED; urgency=medium"
            echo # newline
            release_type="unreleased"
        elif [[ -z $line ]]; then
            if [[ -n $version ]]; then
              do_by_line "$release_date"
            elif [[ "${release_type}" == "unreleased" ]]; then
              do_by_line "$(date -R)"
            fi
        fi
    done < "$input_file"
    echo # newline
} > "$output_file"

echo "Conversion complete. Output saved to $output_file"