#!/usr/bin/perl
while(<>) {
    last if /Yes/;
}

my @arr;

sub pout {
    for (@arr) {
        shift @$_;
        print join ' ', map $_ || '0', @$_;
        print "\n";
    }
}

while (<>) {
    chomp;
    s/^ //;
    s/ +/ /;
    my ($x, $y) = split;
    $arr[$x][$y] = 1;
    $arr[$x-1][$y]++ if $arr[$x-1][$y];
    $arr[$x+1][$y]++ if $arr[$x+1][$y];
    $arr[$x][$y-1]++ if $arr[$x][$y-1];
    $arr[$x][$y+1]++ if $arr[$x][$y+1];
}
pout
