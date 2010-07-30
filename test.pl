#!/usr/bin/perl
use List::Util 'shuffle';
use Time::HiRes qw( clock_gettime clock_getres clock_nanosleep
		      ITIMER_REAL ITIMER_VIRTUAL ITIMER_PROF ITIMER_REALPROF );

my $times = shift;
my $bigtime, $tottime;

for (1..$times) {
    `./gen.pl | tr 5 1 > tmp`;
    $lasttime = clock_gettime(CLOCK_REALTIME);
    `(echo 20 20; cat tmp) | ./nb > tmpout`;
    $tdiff = clock_gettime(CLOCK_REALTIME) - $lasttime;
    if ($bigtime < $tdiff) {
        $bigtime = $tdiff;
        if ($bigtime > 0.01) {
            `(echo 20 20; cat tmp) > slow_tmp`;
        }
    }
    `cat tmpout | ./antibact.pl | tr 5 1 > tmp2`;
    print $res = `diff -u tmp tmp2`;

    if ($res) {
        `mv tmp failed$_ && mv tmp2 failed_out$_`;
    }
    $tottime += $tdiff;
}

$tottime/=$times;

print "Most time spent: $bigtime, average = $tottime\n";
