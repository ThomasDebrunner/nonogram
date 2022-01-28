import numpy as np

h_constraints = [
    [10],
    [2],
    [2,2,1],
    [4,1,1,1],
    [4,2,1,1],
    [2,2,1],
    [2],
    [10, 3],
    [1, 2],
    [1, 1],
    [1, 3],
    [3, 3],
    [2, 3],
    [1, 2, 1],
    [1, 2, 1]
]

v_constraints = [
    [1,2,1],
    [1,4,1],
    [1,4,1],
    [1,2,1],
    [1,1],
    [1,2,1],
    [1,1,2,1],
    [1,1,1,1,3],
    [1,2,1,2],
    [2,2,1,2],
    [6,4,2],
    [1],
    [2,3],
    [8],
    [3]
]


def verify(s, constraint):
    l = len(s)
    x = 0
    for block in constraint:
        if x >= l:
            return False
        while x < l and s[x] == 0:
            x += 1
        b_start = x
        while x < l and s[x] == 1:
            x += 1
        if x - b_start != block:
            return False
    while x < l and s[x] == 0:
        x += 1
    return x == l


def verify_cols(board):
    h, w = board.shape
    for x in range(w):
        if not verify(board[:, x], h_constraints[x]):
            return False
    return True


def dictionary_for_line(row, constraint, idx, d):
    w = len(row)
    if idx >= w:
        if verify(row, constraint):
            d.append(row.copy())
        return
    row[idx] = 1
    dictionary_for_line(row, constraint, idx+1, d)
    row[idx] = 0
    dictionary_for_line(row, constraint, idx+1, d)
    return d


def build_dictionary(constraints, length):
    return [dictionary_for_line(np.zeros(length), constraints[i], 0, []) for i in range(len(constraints))]


def get_fixed(row_dictionary, col_dictionary):
    alls = np.zeros((len(row_dictionary), len(col_dictionary)))
    for y, ops in enumerate(row_dictionary):
        alls[y, :] = np.all(ops, axis=0)
    for x, ops in enumerate(col_dictionary):
        alls[:, x] = np.logical_or(np.all(ops, axis=0), alls[:, x])
    return alls


def clean_dictionary(dictionary, fixed):
    res = []
    for y, ops in enumerate(dictionary):
        l = []
        for opt in ops:
            if np.all(fixed[y, :] == np.logical_and(fixed[y, :], opt)):
                l.append(opt)
        res.append(l)
    return res


def n_ops(dir):
    return sum(len(ops) for ops in dir)


def clean(row_d, col_d, fixed_initial=None):
    while True:
        n_options_initial = n_ops(row_d) + n_ops(col_d)
        if fixed_initial is not None:
            fixed = fixed_initial
            fixed_initial = None
        else:
            fixed = get_fixed(row_d, col_d)
        row_d = clean_dictionary(row_d, fixed)
        col_d = clean_dictionary(col_d, fixed.T)

        n_options_after = n_ops(row_d) + n_ops(col_d)
        if n_options_after == n_options_initial:
            break
    return row_d, col_d


def solve(board, idx, row_dictionary, col_dictionary):
    h, w = board.shape
    if idx >= h:
        return verify_cols(board)
    for row in row_dictionary[idx]:
        board[idx, :] = row
        fixed = get_fixed(row_dictionary, col_dictionary)
        fixed[idx, :] = row
        new_row_dictionary, new_col_dictionary = clean(row_dictionary, col_dictionary, fixed)
        if solve(board, idx+1, new_row_dictionary, new_col_dictionary):
            return True
    return False


def print_board(board):
    h, w = board.shape
    print('----------------')
    for y in range(h):
        for x in range(w):
            if board[y, x] == 1:
                print('█', end='')
            else:
                print(' ', end='')
        print('')


def main():
    print("Building row dictionary...")
    row_dictionary = build_dictionary(v_constraints, len(h_constraints))
    print("Building column dictionary...")
    col_dictionary = build_dictionary(h_constraints, len(v_constraints))

    print("Solving...")
    board = np.zeros((len(v_constraints), len(h_constraints)))
    res = solve(board, 0, row_dictionary, col_dictionary)

    if res:
        print("Solution found:")
        print_board(board)

    else:
        print("No solution found!")
    print("Done.")

if __name__ == '__main__':
    main()