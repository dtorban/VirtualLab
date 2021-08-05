qstat -u dtorban -n -r -1 | sed 's/\// /g' | awk ' NR >=6 {print $12" " }' | tr -d '\n'
