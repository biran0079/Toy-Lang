import string;
import hashtable;
import util;

HT = hashtable;

s = "hello  this is biran    hello again this is biran again";
t = HT.newStringHashTable();
for(w : string.split(s)){
  n = HT.get(t, w);
  if(n==none)
    HT.put(t, w, 1);
  else
    HT.put(t,w,n+1);
}
lst = [];
for(w : HT.keys(t)){
  lst += [[w, HT.get(t, w)]];
}
util.qsort(lst);
print(lst);
