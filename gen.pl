#!/usr/bin/perl
use List::Util 'shuffle';
my @arr;

for (shuffle(map {$a = $_; map [$a, $_], (1..20) } (1..20) )) {
    my ($x, $y) = @$_;
    $arr[$x][$y] = 1;
    $arr[$x-1][$y]++ if $arr[$x-1][$y];
    $arr[$x+1][$y]++ if $arr[$x+1][$y];
    $arr[$x][$y-1]++ if $arr[$x][$y-1];
    $arr[$x][$y+1]++ if $arr[$x][$y+1];
}

for (@arr) {
    shift @$_;
    print join ' ', map $_ || '0', @$_;
    print "\n";
}
