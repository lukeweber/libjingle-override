#!/bin/bash
#
# turntest_wrapper.sh - wraps the turntest and allows us to add a variable number of processes
# @author: Alexander Kudryashov <alexander@tuenti.com>
# @author: Nick Flink <nick@tuenti.com>

##  The following options are provided.
##  -h             help. What you are reading now.
##  -r             restund host. the host where the server is running
##  -c             client host. the host where the client is running
##  -p             port. the port to run the connection through
##  -j             jobs. the number of processes to start
##  -t             threads. the number of threads per processes (recommended 250)
RESTUND_HOST="127.0.0.1"
CLIENT_HOST="127.0.0.1"
TURN_PORT="3478"
JOBS="2"
THREADS="250"
print_usage(){
	echo "$1" >&2
	echo -e "Usage: $0 \n" >&2 
	sed -n '/^## /s/^## //p' $0 >&2
	exit 1
}

validate_optarg(){
	[[ "${OPTARG:0:1}" = "-" ]] && print_usage "-$opt: requires an argument"
}

while getopts "hr:c:p:j:t:" opt
do
	case $opt in
		h ) #help
			print_usage
			;;
		r ) #restund host
			RESTUND_HOST="$OPTARG"
			validate_optarg
			;;
		c ) #client host
			CLIENT_HOST="$OPTARG"
			validate_optarg
			;;
		p ) #port
			TURN_PORT="$OPTARG"
			validate_optarg
			;;
		j ) #jobs
			JOBS="$OPTARG"
			validate_optarg
			;;
		t ) #threads
			THREADS="$OPTARG"
			validate_optarg
			;;
		: )
			print_usage "Option -$OPTARG requires an argument."
			;;
		? )
			if [ "${!OPTIND}" == "--help" ]
			then
				print_usage
			else
				print_usage "Invalid option: -$OPTARG"
			fi
			;;
	esac
done
echo "RESTUND_HOST=$RESTUND_HOST"
echo "CLIENT_HOST=$CLIENT_HOST"
echo "TURN_PORT=$TURN_PORT"
echo "JOBS=$JOBS"
echo "THREADS=$THREADS"

read -p "Press [Enter] key to continue CTRL-C to quit..."
# First possible solution using GNU parallel
# seq 6000 1000 9000 | parallel -j4 ./turntest --port {}

# Second possible solution
# Enable job control

start=$(date '+%s')
set -m
set -e

# In case of Ctrl + C kill all spawned processes
function kill_processes() {
  jobs -p | xargs kill
}

trap kill_processes SIGINT

# Port to start
port=6000
# Jobs to run
jobs=27
# PIDs array
declare -A pids

for n in `seq ${jobs}`;
do
  ./turntest --turn_host=${RESTUND_HOST} --turn_port=${TURN_PORT} --client_host=${CLIENT_HOST} --port ${port} --threads=${THREADS} 2>&1 1>./turn.log.${port} &
  pid=$!
  echo "Backgrounded: $n (pid=$pid)"
  pids[$pid]=$n
  port=$((port + 1000));
done

while [ -n "${pids[*]}" ]; do
  sleep 1
  for pid in "${!pids[@]}"; do
    if ! ps "$pid" >/dev/null; then
      unset pids[$pid]
      echo "unset: $pid"
    fi
  done
  if [ -z "${!pids[*]}" ]; then
    break
  fi
  printf "\rStill waiting for: %s ... " "${pids[*]}"
done

printf "\r%-25s \n" "Done."
printf "Total runtime: %d seconds\n" "$((`date '+%s'` - $start))"

