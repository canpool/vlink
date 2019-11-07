#!/bin/bash
CUR_DIR="$(dirname "$(readlink -f "$0")")"
cd "$CUR_DIR"

test_scripts="$(find ./ -name run_test.sh)";
# echo "$test_scripts";

#
# functions
#
clean()
{
    for i in $test_scripts; do
        sh $i clean
    done
}

run()
{
    for i in $test_scripts; do
        sh $i
        if [ $? != 0 ]; then
            echo "run $i failed"
            return -1
        fi
    done
}

main()
{
	if [ "X$1" = "Xclean" ]; then
		clean
	else
		run
	fi
}

main "$@"