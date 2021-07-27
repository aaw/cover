# N queens for n = 4
# Items r1 - rn represent a queen placed in a row
# Items c1 - cn represent a queen placed in a column
# Items ak represent a queen on the upward diagonal with r+c=k
# Items bk represent a queen on the downward diagonal with r-c+n=k
# Every row and column must be covered, but diagonals are optional

r1 r2 r3 r4 c1 c2 c3 c4 | b3 a3 a4 a8 b4 a5 b2 b5 a7 b6 b7 b1 a2 a6
r1 c1 a2 b4
r1 c2 a3 b3
r1 c3 a4 b2
r1 c4 a5 b1
r2 c1 a3 b5
r2 c2 a4 b4
r2 c3 a5 b3
r2 c4 a6 b2
r3 c1 a4 b6
r3 c2 a5 b5
r3 c3 a6 b4
r3 c4 a7 b3
r4 c1 a5 b7
r4 c2 a6 b6
r4 c3 a7 b5
r4 c4 a8 b4
