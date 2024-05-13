#!/bin/bash

echo "**********************************"
program_output=$(command "./jobCommander" "poll" "queued")
if [ "$program_output" != "No processes queued..." ]; then
    # echo "Program output: $program_output"
    last_line=$(echo "$program_output" | tail -n 1)
    # echo "Last line: $last_line"
    last_job="${last_line%%,*}"
    last_job=${last_job:1}
    # echo "Extracted characters: $last_job"
    last_id=${last_job:4}
    # echo "Extracted characters: $last_id"
    int_id=$((last_id))
    # echo "int: $int_id"
    for ((i = 1; i <= int_id; i++))
    do
        # echo "Number: $i"
        temp_jobID="job_$i"
        # echo "$temp_jobID"
        ./jobCommander stop "$temp_jobID"
    done
fi

echo "       -----       "

program_output=$(command "./jobCommander" "poll" "running")
echo "$program_output"
if [ "$program_output" != "No processes running..." ]; then
    # echo "Program output: $program_output"
#     last_line=$(echo "$program_output" | tail -n 1)
#     last_job="${last_line%%,*}"
#     last_job=${last_job:1}
#     last_id=${last_job:4}
#     int_id=$((last_id))
#     for ((i = 1; i <= int_id; i++))
#     do
#         temp_jobID="job_$i"
#         ./jobCommander stop "$temp_jobID"
#     done
fi
echo "**********************************"




# No processes running...