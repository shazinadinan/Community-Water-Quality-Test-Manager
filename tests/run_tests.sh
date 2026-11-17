#!/usr/bin/env bash
# Automated tests for CA1 and CA2.  Run from the repository root:
#   bash tests/run_tests.sh            run all checks and compare with tests/expected/
#   bash tests/run_tests.sh --update   regenerate tests/expected/ (only after you REVIEWED the output)
# rand() values depend on the C library (expected files were made with g++ on Linux/glibc).
cd "$(dirname "$0")/.." || exit 1
UPDATE=0; [ "$1" = "--update" ] && UPDATE=1
mkdir -p build tests/tmp tests/actual; rm -rf tests/tmp/*
PASS=0; FAIL=0
ok(){ echo "PASS  $1"; PASS=$((PASS+1)); }
bad(){ echo "FAIL  $1"; FAIL=$((FAIL+1)); }
check(){ if grep -q -- "$3" "tests/actual/$2"; then ok "$1"; else bad "$1   (not found: $3)"; fi; }
build(){ if g++ -std=c++17 -Wall -Wextra -pedantic -o "build/$1" "src/$1/main.cpp" 2>"tests/actual/$1.build.txt" && [ ! -s "tests/actual/$1.build.txt" ]; then ok "BUILD-$1 compiles with no warnings"; else bad "BUILD-$1 (see tests/actual/$1.build.txt)"; fi; }
run(){ name=$1; bin=$2; input=$3; shift 3; "build/$bin" "$@" < "tests/inputs/$input" > "tests/actual/$name.out" 2>&1; echo "rc=$?" >> "tests/actual/$name.out"; }
build ca1; build ca2
# ---- CA1 ----
run ca1 ca1 ca1.txt --seed 3
check "CA1-01 menu starts and exits cleanly"               ca1.out "Goodbye"
check "CA1-02 duplicate source ID rejected"                ca1.out "Duplicate source ID"
check "CA1-03 duplicate sample ID rejected"                ca1.out "Duplicate sample ID"
check "CA1-04 invalid calendar date rejected"              ca1.out "Invalid date"
check "CA1-05 out-of-range pH asked again"                 ca1.out "Enter a number from 0.0 to 14.0"
check "CA1-06 unknown source rejected"                     ca1.out "Source not found"
check "CA1-07 unknown sample reported"                     ca1.out "Sample not found"
check "CA1-08 reading above limit flagged"                 ca1.out "REQUIRES ATTENTION"
check "CA1-09 simulated data is labelled"                  ca1.out "SIMULATED DATA"
check "CA1-10 static counter reported"                     ca1.out "static counter): 3"
check "CA1-11 recursion shows location path"               ca1.out "Community > North Ward > Lake Area"
# ---- CA2 (two runs: the second run must see the data of the first) ----
run ca2_s1 ca2 ca2_s1.txt --seed 5 --data tests/tmp/ca2
run ca2_s2 ca2 ca2_s2.txt --seed 5 --data tests/tmp/ca2
check "CA2-01 first run starts with no data"               ca2_s1.out "Loaded 0 source(s) and 0 sample(s)"
[ "$(stat -c %s tests/tmp/ca2/samples.dat)" = "112" ] && ok "CA2-02 samples.dat = 2 records x 56 bytes" || bad "CA2-02 samples.dat size"
[ "$(stat -c %s tests/tmp/ca2/sources.dat)" = "44" ] && ok "CA2-03 sources.dat = 1 record x 44 bytes" || bad "CA2-03 sources.dat size"
check "CA2-04 seekp update overwrites record #0"           ca2_s1.out "Record #0 overwritten"
check "CA2-05 seekg search finds record #0"                ca2_s1.out "Found at record #0"
check "CA2-06 missing record reported"                     ca2_s1.out "Not found in file"
check "CA2-07 save-all message"                            ca2_s1.out "Saved 2 sample(s)"
[ -s tests/tmp/ca2/report.txt ] && ok "CA2-08 text report file created" || bad "CA2-08 report.txt missing"
grep -q "sample 1001 updated" tests/tmp/ca2/log.txt && ok "CA2-09 log.txt appended (ios::app)" || bad "CA2-09 log.txt"
check "CA2-10 exit message"                                ca2_s1.out "Thank you for using the system"
check "CA2-11 data survives restart (2 samples)"           ca2_s2.out "Loaded 1 source(s) and 2 sample(s)"
check "CA2-12 updated values persisted after restart"      ca2_s2.out "pH  6.0 | Turb   8.0 | Hard    350"
check "CA2-13 update can be cancelled"                     ca2_s2.out "Update cancelled"
check "CA2-14 summary shows live-object counter"           ca2_s2.out "objects alive"
for f in ca1 ca2_s1 ca2_s2; do
  if [ $UPDATE = 1 ]; then cp "tests/actual/$f.out" "tests/expected/$f.out"; continue; fi
  if diff -q "tests/actual/$f.out" "tests/expected/$f.out" >/dev/null 2>&1; then ok "EXPECTED-$f identical to saved output"; else bad "EXPECTED-$f differs from tests/expected/$f.out"; fi
done
echo; echo "Result: $PASS passed, $FAIL failed"; [ $FAIL = 0 ]
