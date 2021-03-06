#!/usr/bin/env zsh

WHITE="\e[0m"
GREEN="\e[32m"
RED="\e[31m"

EXE="$1"

if [ -z "${EXE}" ]; then
  echo -ne "${RED}Path to compiler expected as first argument!${WHITE}\n"
  exit 1
fi

for directory in */; do
  directory=${directory%*/}
  testcount=$(ls -1q "$directory"/**/*.mf | wc -l)

  echo -ne "--- Testing $directory with $testcount tests ---\n\n"

  successes=0
  fails=0
  for f in $directory/**/*.mf; do  
    posneg=$(echo -ne "$f" | awk -F/ '{print $(NF-1)}')
    testname=$(echo -ne "$f" | awk -F/ '{print $NF}')
    echo -ne "$posneg test: $testname ...."
    TESTOUT=$(exec ${directory}/invoke.sh ${EXE} "$f")
    TESTRES=$(echo -ne "${TESTOUT}" | grep '^' | perl -pe 's/\033\[[\d;]*m//g' | diff --suppress-common-lines -y ${f%.*}.txt - | wc -l)

    store_file=$(basename ${testname%.*})
    store="$directory/$posneg/fails/$store_file.res"
    if (( ${TESTRES} > 0 )); then
      echo -ne " ${RED}FAIL${WHITE} with ${TESTRES} differing lines of output."
      fails=$(( fails + 1 ))

      mkdir -p "$directory/$posneg/fails"
      touch "$store"

      COLUMW=$(echo -ne "${f%.*}.txt" | wc -c)
      echo $(echo -ne "${TESTOUT}" | grep '^' | perl -pe 's/\033\[[\d;]*m//g' | diff --width=$COLUMW --suppress-common-lines -y ${f%.*}.txt -) > "$store"

      echo -ne " -- For details: cat $store\n"
    else
      echo -ne "${GREEN}SUCCESS${WHITE}.\n"
      successes=$(( successes + 1 ))

      # delete any previous fails
      if [ -f $store ]; then
        rm $store
      fi
    fi
  done
  SUCCCOL="$WHITE"
  if (( ${successes} > 0 )); then
    SUCCCOL="$GREEN"
  fi
  FAILCOL="$GREEN"
  if (( ${fails} > 0 )); then
    FAILCOL="$RED"
  fi
  echo -ne "\n--- Tested $directory (${SUCCCOL}$successes${WHITE} + ${FAILCOL}$fails${WHITE} = $testcount) ---\n"
done

