echo "*********************"
for f in test/*
do
  [[ $f == *.tl ]] || continue
  out=$f.ast
  if ./parser $@ $f | diff - $out  
  then
    echo "$f pass"
  else
    echo "$f fail"
  fi
done
echo "*********************"
