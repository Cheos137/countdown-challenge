# countdown-challenge

On 2023/01/26, the YouTuber `Another Roof` proposed a challenge regarding the british Game Show _Countdown_.
Details are outline in their video available at `https://youtu.be/X-7Wev90lw4`.

This implementation is made in `C`, and is currently not able to solve the problem within a reasonable time frame.
All approaches made here will be single-threaded, the program can be executed multiple time in parallel to compute independent parts of the solution.

There are more optimizations i plan to implement - if you can find any more, i'd be happy to hear them.

## Building

just use your favourite C-Compiler (-Ofast and/or similar compiler optimizations are something you probably want to enable, too).

`countdown.c` the 'original' attempt at solving by following the provided python script rather closely (and implementing most features of the python script).
`countdown_clean.c` a stripped down version of `countdown.c`, contains more optimizations than it and is the one being currently worked at.
