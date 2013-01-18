import hashtable;

HT = hashtable;

t = HT.newStringHashTable();
HT.put(t, "hello", "world");
print(HT.get(t,"hello"));
HT.put(t, "hello", "toy-lang");
print(HT.get(t,"hello"));
HT.put(t, "123", "321");
print(HT.get(t,"123"));
print(HT.get(t,"333"));
print(HT.get(t,"333")==none);
