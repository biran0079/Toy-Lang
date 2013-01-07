for f in test/*
do
  [[ $f == *.tl ]] || continue
  dot=$f.dot
  ./tl -d $f 
  dot -Tjpg -o $f.jpg $dot || break
  echo $f.jpg generated
done
