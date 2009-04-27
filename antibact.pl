#!/usr/bin/perl
while(<>) {
    last if /Yes/;
}

my @arr;

sub pout {
    for (@arr) {
        print join ' ', map $_ || '0', @$_;
        print "\n";
    }
}

while (<>) {
    chomp;
    my ($x, $y) = split;
    $arr[$x][$y] = 1;
    $arr[$x-1][$y]++ if $arr[$x-1][$y];
    $arr[$x+1][$y]++ if $arr[$x+1][$y];
    $arr[$x][$y-1]++ if $arr[$x][$y-1];
    $arr[$x][$y+1]++ if $arr[$x][$y+1];
    print; print "\n"; pout; print " ||\n \\/\n";
}
