package mix_tests;

use strict;
use warnings;
use vars qw(@EXPORT @EXPORT_OK @ISA);

use Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(variegate_list combine_list combine_nk totalcombs);
@EXPORT_OK = @EXPORT;

sub variegate_list($@);
sub combine_list($@);

sub variegate_list($@) {
	my ($amount, @items) = @_;
	my @result;

	$amount == 1 and return map { [ $_ ] } @items;

	foreach my $idx ( 0 .. $#items ) {
		my @copy = @items;
		my @round_item = splice( @copy, $idx, 1 );
		my @round_list = variegate_list( $amount - 1, @copy );
		foreach my $sublist (@round_list) {
			push( @result, [ @round_item, @{$sublist} ] );
		}
	}

	return @result;
}

sub totalcombs($$){
        my ($n, $k) = @_;
	my $c = 1;
        $k > $n and return 0;
        for my $d ( 1 .. $k ) {
                $c *= $n--;
                $c /= $d;
                $c = int($c);
        }
	return $c;
}

sub combine_nk($$) {
        my ($n, $k) = @_;
        my @indx;
        my @result;

        @indx = map { $_ } ( 0 .. $k - 1 );

LOOP:
        while( 1 ) {
                my @line = map { $indx[$_] } ( 0 .. $k - 1 );
                push( @result, \@line ) if @line;
                for( my $iwk = $k - 1; $iwk >= 0; --$iwk ) {
                        if( $indx[$iwk] <= ($n-1)-($k-$iwk) ) {
                                ++$indx[$iwk];
                                for my $swk ( $iwk + 1 .. $k - 1 ) {
                                        $indx[$swk] = $indx[$swk-1]+1;
                                }
                                next LOOP;
                        }
                }
                last;
        }

        return @result;
}

sub combine_list($@) {
	my ($amount, @items) = @_;
	my @result;

	$amount == 1 and return map { [ $_ ] } @items;

	foreach my $idx ( 0 .. $#items ) {
		my @copy = @items;
		my @round_item = splice( @copy, $idx, 1 );
		my @round_list = combine_list( $amount - 1, @copy );
		foreach my $sublist (@round_list) {
			push( @result, [ @round_item, @{$sublist} ] );
		}
	}

	my @sorted = map { [ sort { $a <=> $b } @{$_} ] } @result;
	my %uniq_sorted;
	foreach my $si (@sorted) {
		my $si_key = join(",", @{$si});
		$uniq_sorted{$si_key} = $si;
	}
	@result = values(%uniq_sorted);

	return @result;
}

1;
