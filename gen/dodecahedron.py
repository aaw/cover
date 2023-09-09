#!/usr/bin/python3

# Solution to https://puzzling.stackexchange.com/questions/122258/a-colorful-dodecahedron.
#
# To see the solution, run this program to generate an input file and pass that input file
# to the xcc binary in this repo. For example, from the top directory of this repo:
#
#  $ make
#  $ python3 examples/dodecahedron/dodecahedron.py > /tmp/dodecahedron.xcc
#  $ ./bin/xcc /tmp/dodcecahedron.xcc
#
# Colored pentagons are labeled a-l as in https://puzzling.stackexchange.com/a/122271/84078.
#
# This xcc program labels the 2-D dodecahedron net with edges e1-e30 and faces f1-f12, then
# creates an option for each rotation of each pentagon on each face. The 'colors'
# y, b, yb, by, yby, and byb are used to constrain edge matches, where y and b stand for
# yellow and blue. For example, starting from the base of pentagon 'a' and following the
# edges clockwise, we see edge colors yby, y, y, yb, and by. This corresponds to the
# encoding of pentagon 'a' below as ['yby', 'y', 'y', 'yb', 'by'].
#
# The only real wrinkle is that the color chosen for a particular edge in one face must
# match the _reverse_ coloring in the other face the edge intersects. Colorings y, b, yby,
# and byb don't cause any issues because they're palindromes. But colorings yb and by must
# be handled carefully. In our enumeration of the edges along each face of the Dodecahedron
# below, we give each edge a canonical face and reverse the coloring of any by or yb edge
# if it appears in a non-canonical face.

# A list of faces of the Dodecahedron and their constituent edges, in clockwise order. Each
# edge is labeled as True (canonical) or False (non-canonical) as described above.
FACES = {
    'f1':  [('e1', True),   ('e2', True),   ('e3', True),   ('e4', True),   ('e5', True)  ],
    'f2':  [('e2', False),  ('e6', True),   ('e7', True),   ('e8', True),   ('e9', True)  ],
    'f3':  [('e4', False),  ('e10', True),  ('e11', True),  ('e12', True),  ('e13', True) ],
    'f4':  [('e3', False),  ('e9', False),  ('e14', True),  ('e15', True),  ('e10', False)],
    'f5':  [('e8', False),  ('e16', True),  ('e17', True),  ('e18', True),  ('e14', False)],
    'f6':  [('e11', False), ('e15', False), ('e18', False), ('e19', True),  ('e20', True) ],
    'f7':  [('e19', False), ('e17', False), ('e21', True),  ('e22', True),  ('e23', True) ],
    'f8':  [('e12', False), ('e20', False), ('e23', False), ('e24', True),  ('e25', True) ],
    'f9':  [('e22', False), ('e26', True),  ('e27', True),  ('e28', True),  ('e24', False)],
    'f10': [('e7', False),  ('e29', True),  ('e26', False), ('e21', False), ('e16', False)],
    'f11': [('e5', False),  ('e13', False), ('e25', False), ('e28', False), ('e30', True) ],
    'f12': [('e1', False),  ('e30', False), ('e27', False), ('e29', False), ('e6', False) ],
}

# All twelve colored pentagons and their edge colorings.
PENTAGONS = {
    'a': ['yby', 'y', 'y', 'yb', 'by'],
    'b': ['yby', 'yb', 'by', 'y', 'y'],
    'c': ['byb', 'by', 'y', 'y', 'yb'],
    'd': ['yby', 'y', 'yb', 'by', 'y'],
    'e': ['yby', 'yb', 'b', 'by', 'y'],
    'f': ['yby', 'y', 'yb', 'b', 'by'],
    'g': ['byb', 'b', 'b', 'by', 'yb'],
    'h': ['byb', 'by', 'yb', 'b', 'b'],
    'i': ['yby', 'yb', 'b', 'b', 'by'],
    'j': ['byb', 'b', 'by', 'yb', 'b'],
    'k': ['byb', 'by', 'y', 'yb', 'b'],
    'l': ['byb', 'b', 'by', 'y', 'yb'],
}

# Generates all n rotations of a list of length n.
def rotations(x):
    for i in range(len(x)):
        yield x[i:] + x[:i]

# Example of what rotations generates:
assert([r for r in rotations([1,2,3,4])] == [[1,2,3,4], [2,3,4,1], [3,4,1,2], [4,1,2,3]])

# Return an edge color, possibly re-oriented to take into account a non-canonical edge.
def orient(color, canonical):
    if color == 'by' and not canonical: return 'yb'
    if color == 'yb' and not canonical: return 'by'
    return color

if __name__ == '__main__':
    # Faces and pentagons are primary items; we want to choose exactly one face and one
    # matching of a rotation of a pentagon to that face's edges. Edges are secondary
    # items, since we want them colored consistently. This generates an input that can
    # be solved with xcc.
    faces = ['f{}'.format(i) for i in range(1,13)]
    pentagons = [ch for ch in 'abcdefghijkl']
    edges = ['e{}'.format(i) for i in range(1,31)]
    print(' '.join(faces + pentagons) + " | " + ' '.join(edges))
    for fid, face in FACES.items():
        for pid, pentagon in PENTAGONS.items():
            for prot in rotations(pentagon):
                print('{} {} {}:{} {}:{} {}:{} {}:{} {}:{}'.format(
                    fid, pid,
                    face[0][0], orient(prot[0], face[0][1]),
                    face[1][0], orient(prot[1], face[1][1]),
                    face[2][0], orient(prot[2], face[2][1]),
                    face[3][0], orient(prot[3], face[3][1]),
                    face[4][0], orient(prot[4], face[4][1])))
