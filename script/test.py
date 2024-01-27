#!/usr/bin/python3
import argparse
import os
import re
import subprocess
import sys

def sig(arr):
    sorted_arr = sorted(sorted(element) if isinstance(element, list) else element
                        for element in arr)
    return hash(tuple(hash_array(element) if isinstance(element, list) else element
                      for element in sorted_arr))

class Problem:
    def __init__(self, primary, secondary, items, nsols):
        self.primary = primary
        self.secondary = secondary
        self.items = items
        self.nsols = nsols

    def verify(self, items):
        primaries = set()
        secondaries = {}
        for item in items:
            if self.items.get(sig(item)) is None:
                raise Exception('Item [{}] not in original list of items'.format(' '.join(item)))
            for k,v in self.make_color_map(item).items():
                if k in self.primary:
                    if k in primaries:
                        raise Exception('Primary {} used more than once in solution'.format(k))
                    primaries.add(k)
                else:
                    if secondaries.get(k, v) != v:
                        raise Exception('Secondary {} chosen with inconsistent colors'.format(k))
                    secondaries[k] = v
        extra = primaries - self.primary
        unused = self.primary - primaries
        if len(extra) > 0:
            raise Exception('Extra primary options in solution: {}'.format(extra))
        if len(unused) > 0:
            raise Exception('Missing primary options in solution: {}'.format(unused))
        return True

    def make_color_map(self, item):
        m = {}
        for it in item:
            c = it.split(':')
            if m.get(c[0]) is not None:
                raise Exception('duplicate option {} in item {}'.format(c[0], item))
            color = None
            if len(c) > 1:
                color = c[1]
            m[c[0]] = color
        return m

def parse_input_file(input_filename):
    accum = ''
    cleaned = []
    nsols = -1
    with open(input_filename) as f:
        for line in f.readlines():
            clean = line.split('//')[0].strip()
            if len(clean) == 0:
                m = re.match('// solutions: (\\d+)', line.strip())
                if m:
                    nsols = int(m.groups()[0])
                continue
            accum += clean
            if accum.endswith('\\'):
                accum = accum[:-1]
                continue
            else:
                cleaned.append(accum)
                accum = ''
    options, items = cleaned[0], cleaned[1:]
    items = [[i.strip() for i in item.split()] for item in items]
    ps = options.split('|')
    primary = [o.strip() for o in ps[0].strip().split()]
    secondary = []
    if len(ps) > 1:
        secondary = [o.strip() for o in ps[1].strip().split()]
    return Problem(set(primary), set(secondary), dict((sig(i), i) for i in items), nsols)

def run_subprocess(executable, test_file):
    try:
        process = subprocess.Popen([executable, test_file], stdout=subprocess.PIPE, text=True, bufsize=1)
        with process.stdout:
            for line in process.stdout:
                yield line
    except subprocess.CalledProcessError as e:
        print("Error:", e)

def run_test(executable, test_file, problem):
    parsing = False
    verified_solutions = 0
    for line in run_subprocess(executable, test_file):
        if line.strip().endswith('Solution:'):
            parsing = True
            items = []
        elif parsing and len(line.strip()) == 0:
            parsing = False
            try:
                problem.verify(items)
                verified_solutions += 1
            except Exception as e:
                print('Invalid solution:')
                for item in items:
                    print('  {}'.format(' '.join(item)))
                print(e)
        elif parsing:
            raw_sol = ':'.join(line.split(':')[1:])
            items.append([sol.strip() for sol in raw_sol.strip().split(' ')])
    if verified_solutions != problem.nsols:
        print('Expected {} solutions, got {}.'.format(problem.nsols, verified_solutions))

def find_root_dir():
    cwd = os.getcwd()
    while not os.path.isfile(os.path.join(cwd, "Makefile")):
        parent = os.path.dirname(cwd)
        if parent == cwd:
            raise FileNotFoundError("Makefile not found in any parent directory.")
        cwd = parent
    return cwd

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Example of argparse with one string argument')
    parser.add_argument('-b', '--binary', type=str, choices=['xc','xcc','mcc','dc'], default='xc', help='The set cover binary to test')
    parser.add_argument('-p', '--pattern', type=str, help='Regex for matching file patterns')
    args = parser.parse_args()
    if args.binary == 'mcc':
        print('mcc not yet supported')
        sys.exit(-1)
    extensions = {
        'xc': ['xc'],
        'xcc': ['xc','xcc'],
        'dc': ['xc','xcc'],
        'mcc': ['xc','xcc','mcc'],
    }
    root = find_root_dir()
    test_files = []
    test_dir = os.path.join(root, 'test')
    for ext in extensions[args.binary]:
        test_files += [os.path.join(test_dir,fname) for fname in os.listdir(os.path.join(root,'test')) if fname.endswith(ext)]
        if args.pattern:
            test_files = [tf for tf in test_files if re.search(args.pattern, tf)]
    path_to_binary = os.path.join(root,'bin', args.binary)

    print('Testing {} against test files with extensions {} in {}'.format(args.binary, extensions[args.binary], test_dir))
    for fname in test_files:
        print('Running [{} {}]'.format(args.binary, os.path.basename(fname)))
        problem = parse_input_file(fname)
        run_test(path_to_binary, fname, problem)
