# Create Sudoko puzzle of size n*n

import random
import sys

def main():
    if len(sys.argv) != 2:
        print("Usage: python sudoku_gen.py n")
        sys.exit(1)

    n = int(sys.argv[1])
    
    # Create a list of numbers 1 to n
    nums = list(range(1, n + 1))

    # Create a list of n lists
    board = [[0] * n for i in range(n)]

    # Fill the diagonal n x n boxes
    for i in range(0, n, int(n ** 0.5)):
        for j in range(0, n, int(n ** 0.5)):
            random.shuffle(nums)
            for k in range(int(n ** 0.5)):
                for l in range(int(n ** 0.5)):
                    board[i + k][j + l] = nums[k * int(n ** 0.5) + l]

    # Fill the remaining boxes row by row
    for i in range(n):
        random.shuffle(nums)
        for j in range(n):
            board[i][j] = nums[j]

    # Print the board
    for i in range(n):
        for j in range(n):
            print(board[i][j], end=" ")
        print()

if __name__ == "__main__":
    main()