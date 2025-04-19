import sys
import max2870

ce = 13
le = 10
f_ref = 100000000

vc0 = max2870.max2870(ce, le, f_ref)
x = int(sys.argv[1])
#print(x)
print(f"Частота : {x} MHz", );
r = vc0.setFrequency(x * 1000)
print(f"Частота встановлена: {r/1000} MHz", );
print(vc0)
