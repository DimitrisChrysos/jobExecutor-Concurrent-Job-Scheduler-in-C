#!/bin/bash

# get the output of the command "/jobCommander poll queued" to find the last id existing in the waiting queue
# and then stop all the jobIDs until we reach the last ID  
program_output=$(command "./jobCommander" "poll" "queued")
if [ "$program_output" != "No processes queued..." ]; then
    last_line=$(echo "$program_output" | tail -n 1) # get the last line
    last_job="${last_line%%,*}" # get a substring (until the first "," is reached) of the string
    last_job=${last_job:1}  # get the last jobID
    last_id=${last_job:4}
    int_id=$((last_id)) # get the last ID 
    for ((i = 1; i <= int_id; i++)) # stop all the jobIDs until we reach the last ID 
    do
        temp_jobID="job_$i"
        ./jobCommander stop "$temp_jobID"
    done
fi

# get the output of the command "/jobCommander poll running" to find the last id existing in the running queue
# and then stop all the jobIDs until we reach the last ID  
program_output=$(command "./jobCommander" "poll" "running")
if [ "$program_output" != "No processes running..." ]; then
    last_line=$(echo "$program_output" | tail -n 1) # get the last line
    last_job="${last_line%%,*}" # get a substring (until the first "," is reached) of the string
    last_job=${last_job:1}  # get the last jobID
    last_id=${last_job:4}
    int_id=$((last_id)) # get the last ID 
    for ((i = 1; i <= int_id; i++)) # stop all the jobIDs until we reach the last ID 
    do
        temp_jobID="job_$i"
        ./jobCommander stop "$temp_jobID"
    done
fi