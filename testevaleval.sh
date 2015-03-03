echo "*********************"
for f in test/*
do
  [[ $f == *.tl ]] || continue
  out=$f.out
  if ./tl $@ tl.tl tl.tl $f | diff - $out  
  then
    echo "$f pass"
  else
    echo "$f fail"
  fi
done
echo "*********************"