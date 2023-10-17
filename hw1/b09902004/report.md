# TCG2023 HW1

b09902004 資工四 郭懷元

## How to Compile

Use this command to compile the project:

```
make
```

The source codes will be compiled to a binary called `solve`.

`solve` takes input from stdin and writes the solution to stdout. So one can use it this way:

```
./solve < testcases/11.in > solution_11.out
```

## Algorithm, Heuristic, Tricks

### DFID

Because I only expected DFID to finish the easiest test cases under 5 seconds, I didn't put efforts into move ordering.

I followed the pseudo code in the slides to implement DFID, with the only difference being using recursion instead of maintaining a stack myself.

### A*

I followed the pseudo code in textbook to implement A* algorithm.

I made a small change by using heap and hash table to achieve the effect of modifiable priority queue. Because in this game particular, the first time a game board state is found reachable during A*, it is one of the shorted path to reach that state. Therefore we can use a hash table to record all board states that we have seen (but might still be in the priority queue).

### A* heuristic

I only came up with one heuristic. It is as follows:

1. If the goal piece is specified, the heuristic $h$ is the chessboard distance ( $\max{(|x_1 - x_2|, |y_1 - y_2|)}$ ) between the goal piece and the goal square.
2. If there are no specified goal piece, the heuristic $h$ is the minimum chessboard distance between any piece and the goal square.

This heuristic is trivially admissable (and also consistent), therefore would yield optimal path.

### Pruning

During a state in a shortest-path solution, any piece should have one of the following "temporary goal":

1. Waiting to be taken by others
2. On the road to taking another piece
3. Heading towards the goal square
4. No purpose, just wandering around
   - They can also just get closer to the goal square

Thus any move should be "meaningful", where meaningful means the moved piece should either:

1. Move closer to the goal square
   - For goal 3 and 4
2. Move closer or stay exactly 1 step to one of the other pieces
   - For goal 1 and 2
   - Taking a piece is seen as reducing the distance to 0
   - "Staying exactly 1 step" is because we might want to be taken by that piece instead of taking it

By this property, we can prune useless branches by not pushing those states into our priority queue. Theoretically this should at least make heap operations faster by reducing number of items. I also pruned states where the goal piece is eaten, which is impossible to finish.

### Other implementation tricks

I extended the `ewn.cpp` and `ewn.h` provided for game-related utilities. Through profiling I found that a significant ratio of time was spent on copying game states, which contains roughly unnecessary 100 integers for the 9*9 board and some shared variable. Therefore, I slimed down the game state class to see if there are any difference.


## Experiment Results

Here are some benchmark results. All are compiled with `c++11` and `O3` optimization. Time limit is 10 seconds.

| Method              | `11.in` | `14.in` | `22.in` | `23.in` | `31.in` | `33.in` |
| ------------------- | ------- | ------- | ------- | ------- | ------- | ------- |
| DFID                | 0.007s  | 3.489s  | DNF     | DNF     | DNF     | DNF     |
| A*                  | 0.010s  | 0.093s  | 0.431s  | 1.092s  | 2.116s  | 0.699s  |
| A* + pruning        | 0.010s  | 0.039s  | 0.326s  | 0.586s  | 1.093s  | 0.555s  |
| A* + pruning + slim | 0.009s  | 0.040s  | 0.243s  | 0.390s  | 0.803s  | 0.432s  |

As the table above shows, pruning can reduce around 25~50% of time, and slim game state can save another 20%.

I also used profiling to check how many states are explored (based on calls to certain function). For test case `31.in`, the algorithm explored around 2M states, and it only explored around 1M states with pruning.

## References

People I discussed with: 陳可邦、陳柏諺、王均倍
