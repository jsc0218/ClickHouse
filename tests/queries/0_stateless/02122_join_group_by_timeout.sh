#!/usr/bin/env bash
# Tags: no-debug

# no-debug: Query is canceled by timeout after max_execution_time,
#           but sending an exception to the client may hang
#           for more than MAX_PROCESS_WAIT seconds in a slow debug build,
#           and test will fail.

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
# shellcheck source=../shell_config.sh
. "$CURDIR"/../shell_config.sh

MAX_PROCESS_WAIT=5

IS_SANITIZER=$($CLICKHOUSE_CLIENT -q "SELECT count() FROM system.warnings WHERE message like '%built with sanitizer%'")
if [ "$IS_SANITIZER" -gt 0 ]; then
    # Query may hang for more than 5 seconds, especially in tsan build
    MAX_PROCESS_WAIT=15
fi

# TCP CLIENT: As of today (02/12/21) uses PullingAsyncPipelineExecutor
### Should be cancelled after 1 second and return a 159 exception (timeout)
timeout -s KILL $MAX_PROCESS_WAIT $CLICKHOUSE_CLIENT --max_execution_time 1 -q \
    "SELECT * FROM
    (
        SELECT a.name as n
        FROM
        (
            SELECT 'Name' as name, number FROM system.numbers LIMIT 2000000
        ) AS a,
        (
            SELECT 'Name' as name2, number FROM system.numbers LIMIT 2000000
        ) as b
        GROUP BY n
    )
    LIMIT 20
    FORMAT Null" 2>&1 | grep -o "Code: 159" | sort | uniq

### Should stop pulling data and return what has been generated already (return code 0)
timeout -s KILL $MAX_PROCESS_WAIT $CLICKHOUSE_CLIENT -q \
    "SELECT a.name as n
     FROM
     (
         SELECT 'Name' as name, number FROM system.numbers LIMIT 2000000
     ) AS a,
     (
         SELECT 'Name' as name2, number FROM system.numbers LIMIT 2000000
     ) as b
     FORMAT Null
     SETTINGS max_execution_time = 1, timeout_overflow_mode = 'break'
    "
echo $?


# HTTP CLIENT: As of today (02/12/21) uses PullingPipelineExecutor
### Should be cancelled after 1 second and return a 159 exception (timeout)
${CLICKHOUSE_CURL} -q --max-time $MAX_PROCESS_WAIT -sS "$CLICKHOUSE_URL&max_execution_time=1" -d \
    "SELECT * FROM
    (
        SELECT a.name as n
        FROM
        (
            SELECT 'Name' as name, number FROM system.numbers LIMIT 2000000
        ) AS a,
        (
            SELECT 'Name' as name2, number FROM system.numbers LIMIT 2000000
        ) as b
        GROUP BY n
    )
    LIMIT 20
    FORMAT Null" 2>&1 | grep -o "Code: 159" | sort | uniq


### Should stop pulling data and return what has been generated already (return code 0)
${CLICKHOUSE_CURL} -q --max-time $MAX_PROCESS_WAIT -sS "$CLICKHOUSE_URL" -d \
    "SELECT a.name as n
          FROM
          (
              SELECT 'Name' as name, number FROM system.numbers LIMIT 2000000
          ) AS a,
          (
              SELECT 'Name' as name2, number FROM system.numbers LIMIT 2000000
          ) as b
          FORMAT Null
          SETTINGS max_execution_time = 1, timeout_overflow_mode = 'break'
    "
echo $?
