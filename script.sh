#!/bin/bash

mutants=0
survived=0
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
        src/program.c -o program.o -c
      exit_code=$?

      if [ $exit_code -eq 0 ]; then
        gcc test/programTest.c program.o test/unity.c -o testExample
        ./testExample >/dev/null
        exit_code=$?

        if [ $exit_code -eq 0 ]; then
          echo "MUTANT SURVIVED $function $rule $position"
          survived=$((survived + 1))
        else
          echo "MUTANT CAUGHT   $function $rule $position"
          rm "$result_directory""$function"_"$rule"_"$position"
        fi

        mutants=$((mutants + 1))
        position=$((position + 1))
      else
        position=-1
      fi
    done
  done <$rules
done <$temp_directory/$functions

killed=$((mutants - survived))

echo "Number of mutants: $mutants, survived: $survived"
if [ $mutants -gt 0 ]; then
  awk -v killed=$killed -v mutants=$mutants 'BEGIN { printf "Mutation score: %.2f%%\n", killed/mutants*100 }'
fi
rm -r $temp_directory
