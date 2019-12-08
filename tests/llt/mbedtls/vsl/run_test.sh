#!/bin/bash
CUR_DIR="$(dirname "$(readlink -f "$0")")"
cd "$CUR_DIR"

TOP_DIR=$CUR_DIR/../../../..
TEST_DIR=$TOP_DIR/tests
THIRD_DIR=$TEST_DIR/3rd

TEST_CASE=test

#
# functions
#
clean()
{
	make distclean
}

run()
{
	# deploy
	sh $THIRD_DIR/deploy_googletest.sh

	# make test
	make clean && make -j8

	./${TEST_CASE}
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