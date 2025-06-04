a = []
for n in range(1, 1000):
    r = bin(n)[2::]
    r = r + f'{r.count('1') % 2}'
    r = r + f'{r.count('1') % 2}'
    if int(r, 2) > 43:
        a.append(int(r, 2))
    
print(min(r))
        

