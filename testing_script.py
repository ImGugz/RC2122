#!/usr/bin/env python3
import requests
import os

# Runs all the tests to a DS server and stores the tests' results in the results folder.
# The result of running test x for item y is stored as follows:
#     result/y/result_test_x.html
# The tests mentioned bellow are repeated for some items to ensure the DS database has everything necessary to run those tests:
#     * 6 -> ensures all user are created
#     * 9 -> ensures all groups are created
#     * 14 -> ensures all user are inserted in groups


DSIP = "193.136.128.104"
DSPORT = 58018

#test_group = 27
all_tests = [[1, 6, 8], [1, 2, 6, 7, 8], [6, 3, 5], [4], [6, 9, 10, 11, 12, 14, 15, 17], [6, 9, 11, 14, 16, 17], [6, 9, 14, 16, 17], [
    6, 9, 12, 13, 14, 15, 17], [6, 9, 14, 20, 21, 22], [6, 9, 14, 23, 24, 25, 26, 30, 32, 34, 36, 38], [27, 28, 29, 31, 33, 35, 37, 39]]

# Test group 27 should have 6, 9, and 14 in the beggining

if(os.path.exists("results")):
    os.system("rm -r -f results")
os.mkdir("results")
for tests in all_tests[-2:]:
    test_group = 16 + all_tests.index(tests)
    if test_group >= 24:
        test_group += 1
    print(f"[*] Starting tests for the item {test_group} [*]")
    if(not os.path.exists(f"results/{test_group}")):
        os.mkdir(f"results/{test_group}")
    for test in tests:
        print(f"\t[*] Starting test {test} [*]")
        while(True):
            try:
                r = requests.get(
                    f"http://tejo.tecnico.ulisboa.pt:59000/index.html?DSIP={DSIP}&DSPORT={DSPORT}&SCRIPT={test}")
                with open(f"results/{test_group}/result_test_{test}.html", 'wb') as f:
                    f.write(r.content)
                break
            except requests.exceptions.ConnectionError:
                continue

    #test_group += 1
    # if(test_group == 24):
     #   test_group += 1
