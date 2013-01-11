for f in ./benchmark/*.tl
do
  echo -n "$f   "
  ./tl $f $@
done
