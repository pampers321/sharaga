def f(x, y, z ,w):
    return ((w or not x) and (w == (not y)) and (not w or z))


print('x y z w f')
for x in range(2):
    for y in range(2):
        for z in range(2):
            for w in range(2):
                if f(x, y, z, w):
                    print(x, y, z, w, int(f(x, y, z, w)))