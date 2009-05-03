#!/usr/bin/perl
use List::Util 'shuffle';

my $times = shift;

for (1..$times) {
    `./gen.pl | tr 5 1 > tmp`;
    `(echo 20 20; cat tmp) | ./nb | ./antibact.pl | tr 5 1 > tmp2`;
    print $res = `diff -u tmp tmp2`;

    if ($res) {
        `mv tmp failed$_ && mv tmp2 failed_out$_`;
    }
}
