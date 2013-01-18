for m in `seq 100 110`
do
  ./test.sh -m $m
done
./test.sh -m 0
