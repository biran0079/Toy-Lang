for f in ./benchmark/*.tl
do
  echo -n "$f   "
  time ./tl $f $@
done
