import sort;
import util;
import list;

a = [1,5,3,2,4];
sort.msort(a);
print(a);

l = util.getRandomList(100);
print(list.isSorted(l));
sort.msort(l);
print(list.isSorted(l));

l = util.getRandomList(100);
print(list.isSorted(l));
sort.qsort(l);
print(list.isSorted(l));

