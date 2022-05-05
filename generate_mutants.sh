#!/bin/bash

mutants=0
position=0
functions='function_names.txt'
rules='rules.txt'
result_directory="result/"
temp_directory="tmp/"

rm -f -r $result_directory
mkdir -p $result_directory
mkdir $temp_directory

gcc -fplugin=./plugin.so -fplugin-arg-plugin-collect_function_names src/program.c

while read function; do
  while read rule; do
    position=0
    while [[ position -ne -1 ]]; do
      gcc -fplugin=./plugin.so -fplugin-arg-plugin-rule="$rule" \
        -fplugin-arg-plugin-target_function="$function" \
        -fplugin-arg-plugin-position="$position" \
        -fplugin-arg-plugin-resultdir="$result_directory" \
        src/program.c -o $result_directory/"$function"-"$rule"-"$position".out
      exit_code=$?

      if [ $exit_code -eq 0 ]; then
        echo "MUTANT GENERATED $function $rule $position"
        position=$((position + 1))
        mutants=$((mutants + 1))
      else
        position=-1
      fi
    done
  done <$rules
done <$temp_directory/$functions

echo "Number of mutants: $mutants"
rm -r $temp_directory
