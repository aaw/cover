#!/usr/bin/python3

patterns = {
    (0,0,
     0,1): "\u250c",  # ┌
    (0,0,
     1,0): "\u2510",  # ┐
    (0,0,
     1,1): "\u2500",  # ─
    (0,0,
     1,2): "\u252c",  # ┬
    (0,1,
     0,0): "\u2514",  # └
    (0,1,
     0,1): "\u2502",  # │
    (0,1,
     0,2): "\u251c",  # ├
    (0,1,
     1,1): "\u2518",  # ┘
    (0,1,
     2,1): "\u2524",  # ┤
    (0,1,
     2,2): "\u2534",  # ┴
    (0,1,
     2,3): "\u253c",  # ┼
}

# Sizing of blocks in terms of box-drawing characters above:
#
# ┌─┐   1 = 3x2
# └─┘
#
# ┌───┐ 2 = 5x3
# │  2│
# └───┘
#
# ┌─────┐ 3 = 7x4
# │     │
# │    3│
# └─────┘
#
# So an nxn block is represented by 2n+1 x n+1 box drawing characters.
